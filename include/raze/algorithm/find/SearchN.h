#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/ProjectedValue.h>

#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct search_n_t : Traits, dispatchable<search_n_t<Traits>> {
    template <source Source, class SizeType, class Value, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        static consteval bool vectorizable() noexcept {
            return std::same_as<std::remove_cvref_t<Value>, vector_value_type> &&
                contiguous_source<source_type> &&
                !traits::is_iterator_volatile_v<iterator_type> &&
                vectorizable_binary_predicate<Predicate, unchecked_iterator_type> &&
                vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        SizeType _count;
        SizeType _current_count = 0;
        const Value& _value;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;
        unchecked_iterator_type _candidate;
        unchecked_iterator_type _result_end;

        constexpr explicit kernel(Source&& source, SizeType cnt, const Value& value,
            Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _count(cnt),
              _value(value),
              _predicate(std::move(pred)),
              _proj(std::move(proj)),
              _candidate(_iterator),
              _result_end(_iterator)
        {}

        constexpr bool exit() const noexcept {
            if (_count <= 0)
                return true;

            if constexpr (contiguous_source<source_type>)
                return (_source.size() / sizeof(vector_value_type)) < static_cast<sizetype>(_count);

            return false;
        }

        constexpr std::ranges::subrange<iterator_type> default_result() const {
            if (_count <= 0)
                return { _source.wrap(_iterator), _source.wrap(_iterator) };

            auto end = std::ranges::next(_iterator, _sentinel);
            return { _source.wrap(end), _source.wrap(end) };
        }

        raze_always_inline constexpr void operator()() {
            auto it = _iterator;
            auto cand = _candidate;
            auto cur = _current_count;

            raze_disable_unrolling
            for (; it != _sentinel; ++it) {
                if (std::invoke(_predicate, std::invoke(_proj, *it), _value)) {
                    if (cur == 0) cand = it;

                    if (++cur == _count) {
                        _candidate = cand;
                        _result_end = std::ranges::next(it);
                        return;
                    }
                }
                else {
                    cur = 0;
                }
            }

            _candidate = it;
            _result_end = it;
        }

        template <vectorizable_tag Tag, class Ptr>
        raze_always_inline SizeType trailing_matches(Ptr low, Ptr high, SizeType carry) {
            auto* p = high;
            SizeType matched_cnt = 0;
            const auto target = Tag(_value);

            while (p - low >= static_cast<std::ptrdiff_t>(Tag::size())) {
                p -= Tag::size();
                const auto not_mask = !_predicate(_proj(vx::load<Tag>(p)), target);

                if (!vx::none_of(not_mask)) {
                    // `find_last_set` gives distance from chunk end to the last 0 == trailing 1-run length.
                    return matched_cnt + static_cast<SizeType>(vx::find_last_set[vx::not_null](not_mask));
                }

                matched_cnt += Tag::size();
            }

            while (p != low) {
                if (!_predicate(_proj(*--p), _value))
                    return matched_cnt;
                ++matched_cnt;
            }

            // Only add `carry` if every single element in [low, high) matched.
            return matched_cnt + carry;
        }

        template <vectorizable_tag Tag, class Ptr>
        raze_always_inline bool stride_search(Ptr ptr, Ptr aligned_end, SizeType cur) {
            // Invariant: [left_bound, ptr) are known matches when `left_bound < ptr`.
            // First window of length `_count` ends at `left_bound + (_count - 1)`.
            auto* left_bound = ptr - cur;
            auto* probe = left_bound + (_count - 1);

            for (;;) {
                if (!_predicate(_proj(*probe), _value)) {
                    left_bound = probe + 1;
                    // Check distance before advancing to never form a pointer > `aligned_end` (UB).
                    if (aligned_end - probe <= _count)
                        break;

                    probe += _count;
                    continue;
                }

                // Don't re-verify [left_bound, ptr) when `left_bound < ptr` — those `cur` elements already matched.
                auto* const min_check = left_bound < ptr ? ptr : left_bound;
                auto* left = probe;

                while (left != min_check && _predicate(_proj(*(left - 1)), _value))
                    --left;

                if (left == min_check && left_bound < ptr)
                    left = left_bound;

                const auto matched = static_cast<SizeType>(probe + 1 - left);
                if (matched == _count) {
                    source_type::from_ptr(_candidate, left);
                    source_type::from_ptr(_result_end, probe + 1);
                    return false;
                }

                auto* right = probe + 1;
                const auto need = static_cast<std::ptrdiff_t>(_count - matched);
                const auto avail = aligned_end - right;
                auto* const limit = right + (need < avail ? need : avail);

                // Extend rightward with SIMD chunks so dense/long runs don't degrade to scalar.
                bool hit_zero = false;
                const auto target = Tag(_value);
                while (limit - right >= static_cast<std::ptrdiff_t>(Tag::size())) {
                    const auto not_mask = !_predicate(_proj(vx::load<Tag>(right)), target);

                    if (!vx::none_of(not_mask)) {
                        right += vx::find_first_set[vx::not_null](not_mask);
                        hit_zero = true;
                        break;
                    }

                    right += Tag::size();
                }

                if (!hit_zero) {
                    while (right != limit && _predicate(_proj(*right), _value))
                        ++right;
                }

                if (static_cast<SizeType>(right - left) == _count) {
                    source_type::from_ptr(_candidate, left);
                    source_type::from_ptr(_result_end, right);
                    return false;
                }

                // Reached `aligned_end` with an unbroken run [left, aligned_end) shorter than `_count`.
                if (right == aligned_end) {
                    const auto tail_cur = static_cast<SizeType>(aligned_end - left);
                    _current_count = tail_cur;
                    source_type::from_ptr(_iterator, aligned_end);

                    if (tail_cur > 0)
                        source_type::from_ptr(_candidate, left);

                    return true;
                }

                // `*right` is a mismatch, so no run of `_count` can cross `right`.
                left_bound = right + 1;
                if (aligned_end - right <= _count)
                    break;

                probe = right + _count;
            }

            // Here `left_bound > ptr` and `aligned_end - left_bound < _count`; only the trailing streak matters.
            const auto tail_cur = trailing_matches<Tag>(left_bound, aligned_end, 0);
            _current_count = tail_cur;
            source_type::from_ptr(_iterator, aligned_end);

            if (tail_cur > 0)
                source_type::from_ptr(_candidate, aligned_end - tail_cur);

            return true;
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);
            auto cur = _current_count; // Invariant: [ptr - cur, ptr) all matched, 0 <= cur < _count.

            if (_count == 1) {
                const auto target = Tag(_value);
                raze_disable_unrolling do {
                    const auto mask = _predicate(_proj(vx::load<Tag>(ptr)), target);

                    if (vx::any_of(mask)) {
                        auto* const match = ptr + vx::find_first_set[vx::not_null](mask);
                        source_type::from_ptr(_candidate, match);
                        source_type::from_ptr(_result_end, match + 1);
                        return false;
                    }

                    advance_bytes(ptr, sizeof(Tag));
                } while (ptr != aligned_end);

                source_type::from_ptr(_iterator, ptr);
                return true;
            }

            // Cannot complete `_count` inside [ptr - cur, aligned_end): only the trailing streak at `aligned_end` matters.
            if (static_cast<SizeType>(aligned_end - ptr) < _count - cur) {
                cur = trailing_matches<Tag>(ptr, aligned_end, cur);
                _current_count = cur;
                source_type::from_ptr(_iterator, aligned_end);

                if (cur > 0)
                    source_type::from_ptr(_candidate, aligned_end - cur);

                return true;
            }

            // For large `_count` (>= 2 vectors and >= 256 bytes), probe every `_count`-th element instead of streaming all cache lines.
            if (_count >= static_cast<SizeType>(Tag::size() * 2) &&
                static_cast<sizetype>(_count) * sizeof(vector_value_type) >= 256)
            {
                return stride_search<Tag>(ptr, aligned_end, cur);
            }

            const auto target = Tag(_value);
            raze_disable_unrolling do {
                const auto mask = _predicate(_proj(vx::load<Tag>(ptr)), target);

                if (vx::none_of(mask)) {
                    cur = 0;
                    advance_bytes(ptr, sizeof(Tag));
                    continue;
                }

                const auto not_mask = !mask;

                if (vx::none_of(not_mask)) {
                    // Written as `_count - cur` to avoid signed overflow; `cur < _count` holds.
                    if (static_cast<SizeType>(Tag::size()) >= _count - cur) {
                        source_type::from_ptr(_candidate, ptr - cur);
                        source_type::from_ptr(_result_end, ptr - cur + _count);
                        return false;
                    }

                    cur += Tag::size();
                    advance_bytes(ptr, sizeof(Tag));
                    continue;
                }

                i32 pos = -1;

                if (cur > 0) {
                    // `vx::none_of(not_mask)` was false, so `not_mask` is guaranteed non-zero (`not_null` is safe).
                    const auto first_zero = vx::find_first_set[vx::not_null](not_mask);

                    if (static_cast<SizeType>(first_zero) >= _count - cur) {
                        source_type::from_ptr(_candidate, ptr - cur);
                        source_type::from_ptr(_result_end, ptr - cur + _count);
                        return false;
                    }

                    cur = 0;
                    // `find_next_set` searches strictly > `pos`, so `pos = 0` cleanly skips a zero at lane 0.
                    pos = first_zero;
                }

                // Mixed vector contains at least one 0, so no interior run can reach >= Tag::size().
                if (_count >= static_cast<SizeType>(Tag::size())) {
                    cur = static_cast<SizeType>(vx::find_last_set[vx::not_null](not_mask));
                    advance_bytes(ptr, sizeof(Tag));
                    continue;
                }

                for (;;) {
                    const auto begin = vx::find_next_set(mask, pos);
                    // Guard against `find_next_set` returning 0 when no bits remain > `pos` (prevents 0 <-> pos loops).
                    if (begin <= pos || begin >= Tag::size())
                        break;

                    // Since `_count >= 2`, an isolated 1 at the last lane can only carry `cur = 1` to the next chunk.
                    if (begin == Tag::size() - 1) {
                        cur = 1;
                        break;
                    }

                    auto end = vx::find_next_set(not_mask, begin);
                    if (end <= begin || end > Tag::size())
                        end = Tag::size();

                    const auto run = static_cast<SizeType>(end - begin);

                    if (run >= _count) {
                        source_type::from_ptr(_candidate, ptr + begin);
                        source_type::from_ptr(_result_end, ptr + begin + _count);
                        return false;
                    }

                    if (end == Tag::size()) {
                        cur = run;
                        break;
                    }

                    pos = end;
                }

                advance_bytes(ptr, sizeof(Tag));
            } while (ptr != aligned_end);

            _current_count = cur;
            source_type::from_ptr(_iterator, ptr);

            if (cur > 0)
                source_type::from_ptr(_candidate, ptr - cur);

            return true;
        }

        constexpr raze_always_inline std::ranges::subrange<iterator_type> result() const {
            return { _source.wrap(_candidate), _source.wrap(_result_end) };
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent,
        class Predicate = std::equal_to<>, class Projection = std::identity,
        class T = projected_value_t<It, Projection>>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent sent, std::iter_difference_t<It> cnt, const T& value,
            Predicate pred = {}, Projection proj = {}) const requires (std::indirectly_comparable<
                It, const T*, Predicate, Projection>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)),
            cnt, value, traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R,
        class Predicate = std::equal_to<>, class Projection = std::identity,
        class T = projected_value_t<std::ranges::iterator_t<R>, Projection>>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, std::ranges::range_difference_t<R> cnt, const T& value,
            Predicate pred = {}, Projection proj = {}) const requires (std::indirectly_comparable<
                std::ranges::iterator_t<R>, const T*, Predicate, Projection>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            cnt, value, traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_subrange
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Searches a range for a consecutive sequence of identical elements.
 *
 * 1) Searches `[first, sent)` for the first contiguous sequence of `cnt` elements each satisfying
 *    `bool(std::invoke(pred, std::invoke(proj, *i), value))`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `search_n(std::ranges::begin(r), std::ranges::end(r), cnt, value, pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>,
 *           class Predicate = std::equal_to<>, class Projection = std::identity >
 *   requires(std::indirectly_comparable<It, const T*, Predicate, Projection>)
 * constexpr std::ranges::subrange<It>
 * search_n( It first, Sent sent, std::iter_difference_t<It> cnt, const T& value,
 *           Predicate pred = {}, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class T = std::ranges::range_value_t<R>,
 *           class Predicate = std::equal_to<>, class Projection = std::identity >
 *   requires(std::indirectly_comparable<std::ranges::iterator_t<R>, const T*, Predicate, Projection>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * search_n( R&& r, std::ranges::range_difference_t<R> cnt, const T& value,
 *           Predicate pred = {}, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `cnt`           - the required number of consecutive matching elements
 * - `value`         - the value to compare against
 * - `pred`          - binary predicate comparing projected elements to `value` (defaults to `std::equal_to<>`)
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` spanning `[found, found + cnt)` for the first matching run,
 * `{first, first}` if `cnt <= 0`, or `{end, end}` if no such sequence exists.
 *
 * ### Complexity
 * Linear: at most `std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`) applications of `pred`
 * and `proj` (sublinear in practice for long runs via strided probing).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Immediately returns `{first, first}` when `cnt <= 0`, and `{end, end}` when a contiguous range is shorter than `cnt`.
 * - Explicit `raze::vx` SIMD provides specialized fast paths for `cnt == 1`, bitmask run scanning for
 *   short/medium runs, and strided probing with SIMD rightward extension for long runs
 *   (`cnt >= 2 * Tag::size()` and at least 256 bytes).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/SearchN.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 2, 3, 2, 2, 2, 4};
 *
 *     auto found = raze::algorithm::search_n(v, 3, 2);
 *     if (!found.empty())
 *         std::cout << "Found run of three 2s at index " << (found.begin() - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Found run of three 2s at index 4
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::search`
 * - `raze::algorithm::find`
 * - `raze::algorithm::adjacent_find`
 * - `raze::algorithm::find_end`
*/
constexpr inline auto search_n = options::function_with_traits<search_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END