#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <algorithm>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct minmax_element_t : Traits, dispatchable<minmax_element_t<Traits>> {
    template <source Source, class Comp, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using result_type = std::ranges::minmax_element_result<iterator_type>;
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
        unchecked_iterator_type _min;
        unchecked_iterator_type _max;
        bool _started = false;

        constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _comp(std::move(comp)),
              _proj(std::move(proj)),
              _min(_iterator),
              _max(_iterator)
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        raze_nodiscard constexpr raze_always_inline result_type default_result() const {
            auto it = _source.wrap(_iterator);
            return { it, it };
        }

        raze_always_inline constexpr void operator()() {
            auto it = _iterator;
            if (it == _sentinel) return;

            auto best_min = _min;
            auto best_max = _max;
            if (!_started) {
                best_min = it;
                best_max = it;
                ++it;
                _started = true;
            }

            raze_novec_loop((; it != _sentinel; ++it), {
                if (std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, *best_min)))
                    best_min = it;
                if (!std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, *best_max)))
                    best_max = it;
            })

            _min = std::move(best_min);
            _max = std::move(best_max);
            _iterator = std::move(it);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            using unsigned_t = typename IntegerForSizeof<value_type>::Unsigned;
            using index_simd_t = vx::simd<unsigned_t, vx::abi_t<Tag>>;

            constexpr auto alignment = align::template policy_for<0, Tag>;
            auto* ptr = std::to_address(_iterator);
            auto* min_ptr = std::to_address(_min);
            auto* max_ptr = std::to_address(_max);
            bool started = _started;

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
                    if constexpr (is_less_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        min_vals = vx::vertical_min(curr_vals, min_vals);
                    else if constexpr (is_greater_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        min_vals = vx::vertical_max(curr_vals, min_vals);
                    else
                        min_vals = vx::select(better_min, curr_vals, min_vals);

                    const auto keep_max = _comp(curr_vals, max_vals);
                    max_idxs = vx::select(vx::convert<unsigned_t>(keep_max), max_idxs, curr_idxs);
                    if constexpr (is_less_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        max_vals = vx::vertical_max(max_vals, curr_vals);
                    else if constexpr (is_greater_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        max_vals = vx::vertical_min(max_vals, curr_vals);
                    else
                        max_vals = vx::select(keep_max, max_vals, curr_vals);

                    advance_bytes(ptr, sizeof(Tag));
                }

                value_type portion_min;
                value_type portion_max;

                if constexpr (is_less_comparator<Comp, value_type>) {
                    portion_min = vx::horizontal_min(min_vals);
                    portion_max = vx::horizontal_max(max_vals);
                }
                else if constexpr (is_greater_comparator<Comp, value_type>) {
                    portion_min = vx::horizontal_max(min_vals);
                    portion_max = vx::horizontal_min(max_vals);
                }
                else {
                    portion_min = vx::fold(min_vals, [&](const auto& x, const auto& y) raze_always_inline_lambda {
                        return vx::select(_comp(y, x), y, x);
                    });
                    portion_max = vx::fold(max_vals, [&](const auto& x, const auto& y) raze_always_inline_lambda {
                        return vx::select(_comp(x, y), y, x);
                    });
                }

                if (!started || std::invoke(_comp, portion_min, *min_ptr)) {
                    typename Tag::mask_type eq_min_mask;
                    if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>)
                        eq_min_mask = (min_vals == portion_min);
                    else {
                        const Tag min_vec(portion_min);
                        eq_min_mask = (!_comp(min_vals, min_vec)) & (!_comp(min_vec, min_vals));
                    }

                    if (vx::any_of(eq_min_mask)) {
                        const auto idx_eq_min = vx::convert<unsigned_t>(eq_min_mask);
                        const auto masked_min = vx::select(idx_eq_min, min_idxs, vx::all_bits<index_simd_t>());
                        const auto best_min_idx = vx::horizontal_min(masked_min);
                        const auto h_pos = vx::find_first_set[vx::not_null](masked_min == best_min_idx);
                        const auto v_pos = static_cast<sizetype>(min_idxs[h_pos]) - 1;

                        auto* cand_min = bytes_pointer_offset(portion_begin,
                            v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));

                        if (!started || std::invoke(_comp, *cand_min, *min_ptr))
                            min_ptr = cand_min;
                    }
                }

                if (!started || !std::invoke(_comp, portion_max, *max_ptr)) {
                    typename Tag::mask_type eq_max_mask;
                    if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>)
                        eq_max_mask = (max_vals == portion_max);
                    else {
                        const Tag max_vec(portion_max);
                        eq_max_mask = (!_comp(max_vals, max_vec)) & (!_comp(max_vec, max_vals));
                    }

                    if (vx::any_of(eq_max_mask)) {
                        const auto idx_eq_max = vx::convert<unsigned_t>(eq_max_mask);
                        const auto masked_max = vx::select(idx_eq_max, max_idxs, vx::zero<index_simd_t>());
                        const auto best_max_idx = vx::horizontal_max(masked_max);
                        const auto h_pos = Tag::size() - 1 - vx::find_last_set[vx::not_null](masked_max == best_max_idx);
                        const auto v_pos = static_cast<sizetype>(max_idxs[h_pos]) - 1;

                        auto* cand_max = bytes_pointer_offset(portion_begin,
                            v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));

                        if (!started || !std::invoke(_comp, *cand_max, *max_ptr))
                            max_ptr = cand_max;
                    }
                }

                started = true;
            }

            _started = started;
            source_type::from_ptr(_iterator, ptr);
            source_type::from_ptr(_min, min_ptr);
            source_type::from_ptr(_max, max_ptr);
        }

        raze_nodiscard constexpr raze_always_inline result_type result() const {
            return { _source.wrap(_min), _source.wrap(_max) };
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

    template <std::forward_iterator It, std::sentinel_for<It> Sent,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::minmax_element_result<It>
    operator()(It first, Sent last, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::minmax_element_result<std::ranges::borrowed_iterator_t<R>>
    operator()(R&& r, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_extreme
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds iterators to both the smallest and largest elements in a range.
 *
 * 1) Finds both the smallest and largest elements in the range `[first, last)` according to
 *    the comparator `comp` after applying the projection `proj`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `minmax_element(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Projection = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
 * constexpr std::ranges::minmax_element_result<It>
 * minmax_element( It first, Sent last, Comp comp = {}, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Comp = std::less<>, class Projection = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::minmax_element_result<std::ranges::borrowed_iterator_t<R>>
 * minmax_element( R&& r, Comp comp = {}, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `comp`          - comparison function object returning `true` if the first argument is less than the second (defaults to `std::less<>`)
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::minmax_element_result` containing:
 * - `min`: iterator to the first smallest element in the range (or `first` if empty).
 * - `max`: iterator to the last largest element in the range (or `first` if empty).
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `2 * max(N - 1, 0)` comparisons in the scalar path, or `O(N / W)` vector comparisons
 * in the explicit SIMD path.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range (1 range).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD tracks both first-min and last-max lane indices in a single pass
 *   (partitioning large inputs for narrow element types to avoid index overflow).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/extreme/MinmaxElement.h>
 *
 * int main() {
 *     const std::vector<int> v{3, 1, -14, 1, 9, -14, 9};
 *
 *     auto [min_it, max_it] = raze::algorithm::minmax_element(v);
 *     std::cout << "Min: " << *min_it << " at " << (min_it - v.begin()) << '\n';
 *     std::cout << "Max: " << *max_it << " at " << (max_it - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Min: -14 at 2
 * Max: 9 at 6
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::minmax`
 * - `raze::algorithm::min_element`
 * - `raze::algorithm::max_element`
*/
constexpr inline auto minmax_element = options::function_with_traits<minmax_element_t>;

__RAZE_ALGORITHM_NAMESPACE_END
