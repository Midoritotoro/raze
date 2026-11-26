#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <algorithm>
#include <initializer_list>
#include <optional>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct minmax_t : Traits, dispatchable<minmax_t<Traits>> {
    template <source Source, class Comp, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using result_type = std::ranges::minmax_result<value_type>;
        using vector_value_type = value_type;
        using align = options::alignments<Traits, 1>;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type> &&
                !traits::is_iterator_volatile_v<iterator_type> &&
                vectorizable_binary_predicate<Comp, unchecked_iterator_type> &&
                vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        raze_no_unique_address source_type _source;
        unchecked_iterator_type _iterator;
        raze_no_unique_address unchecked_sentinel_type _sentinel;
        raze_no_unique_address Comp _comp;
        raze_no_unique_address Projection _proj;
        std::optional<result_type> _value;

        constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _comp(std::move(comp)),
              _proj(std::move(proj))
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        raze_nodiscard constexpr raze_always_inline std::optional<result_type> default_result() const noexcept {
            return std::nullopt;
        }

        raze_always_inline constexpr void operator()(autovectorizable_tag)
            requires(contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>)
        {
            auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);
            if (first == last) return;

            first = align::template assume<0>(first);

            std::conditional_t<traits::lightweight_callable<Comp>, Comp, Comp&> comp = _comp;
            std::conditional_t<traits::lightweight_callable<Projection>, Projection, Projection&> proj = _proj;

            auto start = first;
            if (!_value.has_value()) {
                value_type first_val = *start;
                _value.emplace(first_val, std::move(first_val));
                ++start;
            }

            auto best_min = std::move(_value->min);
            auto best_max = std::move(_value->max);
            const auto len = last - start;

            for (std::remove_const_t<decltype(len)> i = 0; i < len; ++i) {
                if (std::invoke(comp, std::invoke(proj, start[i]), std::invoke(proj, best_min)))
                    best_min = start[i];
                if (!std::invoke(comp, std::invoke(proj, start[i]), std::invoke(proj, best_max)))
                    best_max = start[i];
            }

            _value->min = std::move(best_min);
            _value->max = std::move(best_max);
            source_type::from_ptr(_iterator, last);
        }

        raze_always_inline constexpr void operator()() {
            auto it = _iterator;
            if (it == _sentinel) return;

            if (!_value.has_value()) {
                value_type first_val = *it;
                _value.emplace(first_val, std::move(first_val));
                ++it;
            }

            auto best_min = std::move(_value->min);
            auto best_max = std::move(_value->max);

            raze_novec_loop((; it != _sentinel; ++it), {
                if (std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, best_min)))
                    best_min = *it;
                if (!std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, best_max)))
                    best_max = *it;
            })

            _value->min = std::move(best_min);
            _value->max = std::move(best_max);
            _iterator = std::move(it);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void merge_vectors(const Tag& vmin, const Tag& vmax) {
            value_type cand_min;
            value_type cand_max;

            if constexpr (is_less_comparator<Comp, value_type>) {
                cand_min = vx::horizontal_min(vmin);
                cand_max = vx::horizontal_max(vmax);
            }
            else {
                cand_min = vx::horizontal_max(vmin);
                cand_max = vx::horizontal_min(vmax);
            }

            if (!_value.has_value()) {
                _value.emplace(cand_min, cand_max);
            }
            else {
                if (std::invoke(_comp, cand_min, _value->min))
                    _value->min = cand_min;
                if (!std::invoke(_comp, cand_max, _value->max))
                    _value->max = cand_max;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            constexpr auto alignment = align::template policy_for<0, Tag>;
            auto* ptr = std::to_address(_iterator);

            if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>) {
                const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

                auto vmin = vx::load<Tag>[alignment](ptr);
                auto vmax = vmin;
                advance_bytes(ptr, sizeof(Tag));

                raze_disable_unrolling
                while (ptr != aligned_end) {
                    const auto loaded = vx::load<Tag>[alignment](ptr);

                    if constexpr (is_less_comparator<Comp, value_type>) {
                        vmin = vx::vertical_min(loaded, vmin);
                        vmax = vx::vertical_max(vmax, loaded);
                    }
                    else {
                        vmin = vx::vertical_max(loaded, vmin);
                        vmax = vx::vertical_min(vmax, loaded);
                    }

                    advance_bytes(ptr, sizeof(Tag));
                }

                source_type::from_ptr(_iterator, ptr);
                merge_vectors(vmin, vmax);
            }
            else {
                using unsigned_t = typename IntegerForSizeof<value_type>::Unsigned;
                using index_simd_t = vx::simd<unsigned_t, vx::abi_t<Tag>>;

                constexpr bool needs_chunking = sizeof(unsigned_t) < sizeof(sizetype);
                constexpr sizetype max_portion_bytes = needs_chunking
                    ? static_cast<sizetype>(math::max_limit<unsigned_t>() - 1) * sizeof(Tag)
                    : math::max_limit<sizetype>();

                while (aligned_size > 0) {
                    const auto portion_bytes = needs_chunking
                        ? math::min(max_portion_bytes, aligned_size)
                        : aligned_size;
                    aligned_size -= portion_bytes;

                    auto* portion_begin = ptr;
                    const auto stop_at = bytes_pointer_offset(ptr, portion_bytes);

                    auto min_vals = vx::load<Tag>[alignment](ptr);
                    auto max_vals = min_vals;
                    auto curr_idxs = index_simd_t(static_cast<unsigned_t>(1));
                    auto min_idxs = curr_idxs;
                    auto max_idxs = curr_idxs;

                    advance_bytes(ptr, sizeof(Tag));

                    raze_disable_unrolling
                    while (ptr != stop_at) {
                        ++curr_idxs;
                        const auto curr_vals = vx::load<Tag>[alignment](ptr);

                        const auto better_min = _comp(curr_vals, min_vals);
                        min_idxs = vx::select(vx::convert<unsigned_t>(better_min), curr_idxs, min_idxs);
                        min_vals = vx::select(better_min, curr_vals, min_vals);

                        const auto keep_max = _comp(curr_vals, max_vals);
                        max_idxs = vx::select(vx::convert<unsigned_t>(keep_max), max_idxs, curr_idxs);
                        max_vals = vx::select(keep_max, max_vals, curr_vals);

                        advance_bytes(ptr, sizeof(Tag));
                    }

                    const value_type portion_min = vx::fold(min_vals,
                        [&](const auto& x, const auto& y) raze_always_inline_lambda {
                            return vx::select(_comp(y, x), y, x);
                        });
                    const value_type portion_max = vx::fold(max_vals,
                        [&](const auto& x, const auto& y) raze_always_inline_lambda {
                            return vx::select(_comp(x, y), y, x);
                        });

                    const Tag min_vec(portion_min);
                    const Tag max_vec(portion_max);

                    if (!_value.has_value()) {
                        value_type cand_min = *portion_begin;
                        value_type cand_max = cand_min;

                        const auto eq_min_mask = (!_comp(min_vals, min_vec)) & (!_comp(min_vec, min_vals));
                        if (vx::any_of(eq_min_mask)) {
                            const auto idx_eq_min = vx::convert<unsigned_t>(eq_min_mask);
                            const auto masked_min = vx::select(idx_eq_min, min_idxs, vx::all_bits<index_simd_t>());
                            const auto best_min_idx = vx::horizontal_min(masked_min);
                            const auto h_pos = vx::find_first_set[vx::not_null](masked_min == best_min_idx);
                            const auto v_pos = static_cast<sizetype>(min_idxs[h_pos]) - 1;
                            cand_min = *bytes_pointer_offset(portion_begin,
                                v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));
                        }

                        const auto eq_max_mask = (!_comp(max_vals, max_vec)) & (!_comp(max_vec, max_vals));
                        if (vx::any_of(eq_max_mask)) {
                            const auto idx_eq_max = vx::convert<unsigned_t>(eq_max_mask);
                            const auto masked_max = vx::select(idx_eq_max, max_idxs, vx::zero<index_simd_t>());
                            const auto best_max_idx = vx::horizontal_max(masked_max);
                            const auto h_pos = Tag::size() - 1 - vx::find_last_set[vx::not_null](masked_max == best_max_idx);
                            const auto v_pos = static_cast<sizetype>(max_idxs[h_pos]) - 1;
                            cand_max = *bytes_pointer_offset(portion_begin,
                                v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));
                        }

                        _value.emplace(cand_min, cand_max);
                    }
                    else {
                        if (std::invoke(_comp, portion_min, _value->min)) {
                            const auto eq_min_mask = (!_comp(min_vals, min_vec)) & (!_comp(min_vec, min_vals));
                            if (vx::any_of(eq_min_mask)) {
                                const auto idx_eq_min = vx::convert<unsigned_t>(eq_min_mask);
                                const auto masked_min = vx::select(idx_eq_min, min_idxs, vx::all_bits<index_simd_t>());
                                const auto best_min_idx = vx::horizontal_min(masked_min);
                                const auto h_pos = vx::find_first_set[vx::not_null](masked_min == best_min_idx);
                                const auto v_pos = static_cast<sizetype>(min_idxs[h_pos]) - 1;
                                const auto cand_min = *bytes_pointer_offset(portion_begin,
                                    v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));
                                if (std::invoke(_comp, cand_min, _value->min))
                                    _value->min = cand_min;
                            }
                        }

                        if (!std::invoke(_comp, portion_max, _value->max)) {
                            const auto eq_max_mask = (!_comp(max_vals, max_vec)) & (!_comp(max_vec, max_vals));
                            if (vx::any_of(eq_max_mask)) {
                                const auto idx_eq_max = vx::convert<unsigned_t>(eq_max_mask);
                                const auto masked_max = vx::select(idx_eq_max, max_idxs, vx::zero<index_simd_t>());
                                const auto best_max_idx = vx::horizontal_max(masked_max);
                                const auto h_pos = Tag::size() - 1 - vx::find_last_set[vx::not_null](masked_max == best_max_idx);
                                const auto v_pos = static_cast<sizetype>(max_idxs[h_pos]) - 1;
                                const auto cand_max = *bytes_pointer_offset(portion_begin,
                                    v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));
                                if (!std::invoke(_comp, cand_max, _value->max))
                                    _value->max = cand_max;
                            }
                        }
                    }
                }

                source_type::from_ptr(_iterator, ptr);
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            if (ignore.tail_bytes() == 0) return;

            if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>) {
                constexpr auto alignment = align::template policy_for<0, Tag>;
                auto* ptr = std::to_address(_iterator);
                const auto mask = ignore.mask();
                const Tag fallback(*ptr);
                const auto loaded = vx::select(mask, vx::load<Tag>[ignore][alignment](ptr), fallback);

                advance_bytes(ptr, ignore.tail_bytes());
                source_type::from_ptr(_iterator, ptr);
                merge_vectors(loaded, loaded);
            }
            else {
                (*this)();
            }
        }

        raze_nodiscard constexpr raze_always_inline std::optional<result_type> result() const {
            return _value;
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }

        constexpr void expects() const {
            align::verify(_source);
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<std::ranges::minmax_result<std::iter_value_t<It>>>
    operator()(It first, Sent last, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<std::ranges::minmax_result<std::ranges::range_value_t<R>>>
    operator()(R&& r, Comp comp = {}, Projection proj = {}) const
        requires(!std::same_as<std::remove_cvref_t<R>, std::initializer_list<std::ranges::range_value_t<R>>> &&
            std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <class T, class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<std::ranges::minmax_result<T>>
    operator()(std::initializer_list<T> ilist, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<const T*, Projection>>)
    {
        return this->dispatch(get_source(ilist.begin(), ilist.end()),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_extreme
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Returns both the smallest and largest elements in a range.
 *
 * 1) Finds both the smallest and largest elements in the range `[first, last)` according to
 *    the comparator `comp` after applying the projection `proj`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `minmax(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * 3) Same as (1), operating on `std::initializer_list<T>`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Projection = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
 * constexpr std::optional<std::ranges::minmax_result<std::iter_value_t<It>>>
 * minmax( It first, Sent last, Comp comp = {}, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Comp = std::less<>, class Projection = std::identity >
 *   requires(!std::same_as<std::remove_cvref_t<R>, std::initializer_list<std::ranges::range_value_t<R>>> &&
 *            std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::optional<std::ranges::minmax_result<std::ranges::range_value_t<R>>>
 * minmax( R&& r, Comp comp = {}, Projection proj = {} );
 *
 * // (3) Initializer-list overload
 * template< class T, class Comp = std::less<>, class Projection = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<const T*, Projection>>)
 * constexpr std::optional<std::ranges::minmax_result<T>>
 * minmax( std::initializer_list<T> ilist, Comp comp = {}, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `ilist`         - initializer list with the values to compare
 * - `comp`          - comparison function object returning `true` if the first argument is less than the second (defaults to `std::less<>`)
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::optional<std::ranges::minmax_result<T>>` containing `{min, max}`, or `std::nullopt` if the range is empty.
 * If several elements are equivalent to the smallest, `result->min` is a copy of the first such element.
 * If several elements are equivalent to the largest, `result->max` is a copy of the last such element.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `2 * max(N - 1, 0)` comparisons in the scalar path, or `O(N / W)` vector comparisons
 * in the explicit SIMD path.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`.
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range (1 range).
 * - **Auto-vectorization:** Provides an `autovec` kernel (see @ref autovec_behavior).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD uses hardware `vertical_min`/`vertical_max` with masked tail reduction for
 *   standard comparators (`std::less`/`std::greater`), and lane index tracking to preserve exact first-min
 *   and last-max tie-breaking for custom vectorizable comparators.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/extreme/Minmax.h>
 *
 * int main() {
 *     const std::vector<int> v{3, 1, -14, 1, 5, 9, 2};
 *
 *     if (auto res = raze::algorithm::minmax(v))
 *         std::cout << "Min: " << res->min << ", Max: " << res->max << '\n';
 *
 *     auto il = raze::algorithm::minmax({7, 2, 10, -4, 6});
 *     std::cout << "Init list: " << il->min << " .. " << il->max << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Min: -14, Max: 9
 * Init list: -4 .. 10
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::min`
 * - `raze::algorithm::max`
 * - `raze::algorithm::minmax_element`
*/
constexpr inline auto minmax = options::function_with_traits<minmax_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
