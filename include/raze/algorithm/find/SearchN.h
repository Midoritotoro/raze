#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/math/Math.h>

#include <algorithm>

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

        constexpr explicit kernel(Source&& source, SizeType count, const Value& value,
            Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _count(count),
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
                if (_predicate(_proj(*it), _value)) {
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

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto* const aligned_end = bytes_pointer_offset(ptr, aligned_size);
            auto cur = _current_count; // Trailing match streak immediately preceding `ptr` (0 <= cur < _count)

            raze_disable_unrolling do {
                const auto mask = _predicate(_proj(vx::load<Tag>(ptr)), _value);

                if (vx::none_of(mask)) {
                    cur = 0;
                    advance_bytes(ptr, sizeof(Tag));
                    continue;
                }

                const auto not_mask = !mask;

                if (vx::none_of(not_mask)) {
                    // Combined streak starts at `ptr - cur`
                    if (Tag::size() >= _count - cur) {
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
                    // `vx::none_of(not_mask)` above was false, so `not_mask` has >= 1 set bit -> `[vx::not_null]` is safe.
                    // `first_zero` is both the index of the first 0 and the count of leading 1s: [0, first_zero).
                    const auto first_zero = vx::find_first_set[vx::not_null](not_mask);

                    if (static_cast<SizeType>(first_zero) >= _count - cur) {
                        source_type::from_ptr(_candidate, ptr - cur);
                        source_type::from_ptr(_result_end, ptr - cur + _count);
                        return false;
                    }

                    cur = 0;
                    pos = first_zero;
                }

                for (;;) {
                    const auto begin = vx::find_next_set(mask, pos);
                    if (begin <= pos || begin >= Tag::size())
                        break;

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

            if (cur > 0) source_type::from_ptr(_candidate, ptr - cur);

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
        class T = std::iter_value_t<It>, class Predicate = std::equal_to<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent sent, std::iter_difference_t<It> count, const T& value,
            Predicate pred = {}, Projection proj = {}) const requires (std::indirectly_comparable<
                It, const T*, Predicate, Projection>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)),
            count, value, traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class T = std::ranges::range_value_t<R>, 
        class Predicate = std::equal_to<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, std::ranges::range_difference_t<R> count, const T& value,
            Predicate pred = {}, Projection proj = {}) const requires (std::indirectly_comparable<
                std::ranges::iterator_t<R>, const T*, Predicate, Projection>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            count, value, traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto search_n = options::function_with_traits<search_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END