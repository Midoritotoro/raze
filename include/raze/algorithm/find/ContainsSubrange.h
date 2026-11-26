#pragma once

#include <raze/algorithm/find/Search.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct contains_subrange_t : Traits {
    template <std::forward_iterator It1, std::sentinel_for<It1> Sent1,
        std::forward_iterator It2, std::sentinel_for<It2> Sent2,
        class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(
        It1 first1, Sent1 last1, It2 first2, Sent2 last2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
        requires(std::indirectly_comparable<It1, It2, Predicate, Projection1, Projection2>)
    {
        if (first2 == last2) return true;
        return !algorithm::search[Traits::traits()](std::move(first1), std::move(last1),
            std::move(first2), std::move(last2), traits::fwd_fn(pred), traits::fwd_fn(proj1), 
            traits::fwd_fn(proj2)).empty();
    }

    template <std::ranges::forward_range R1, std::ranges::forward_range R2,
        class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(
        R1&& r1, R2&& r2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
        requires(std::indirectly_comparable<std::ranges::iterator_t<R1>,
            std::ranges::iterator_t<R2>, Predicate, Projection1, Projection2>)
    {
        if (std::ranges::empty(r2)) return true;
        return !algorithm::search[Traits::traits()](r1, r2,
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2)).empty();
    }
};

/**
 * @ingroup raze_algo_subrange
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether a range contains a given contiguous subsequence.
 *
 * 1) Returns `true` if `[first2, last2)` is empty or appears as a contiguous subsequence of `[first1, last1)`
 *    under `pred`, `proj1`, and `proj2`; otherwise returns `false`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `contains_subrange(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::forward_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Predicate = std::equal_to<>,
 *           class Projection1 = std::identity, class Projection2 = std::identity >
 *   requires(std::indirectly_comparable<It1, It2, Predicate, Projection1, Projection2>)
 * constexpr bool contains_subrange( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *                                   Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R1, std::ranges::forward_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Projection1 = std::identity, class Projection2 = std::identity >
 *   requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Projection1, Projection2>)
 * constexpr bool contains_subrange( R1&& r1, R2&& r2,
 *                                   Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - the range of elements to search in (haystack)
 * - `first2`, `last2` - the subsequence to search for (needle)
 * - `r1`              - the range of elements to search in (haystack)
 * - `r2`              - the subsequence to search for (needle)
 * - `pred`            - binary predicate applied to projected elements (defaults to `std::equal_to<>`)
 * - `proj1`, `proj2`  - projections to apply to elements of the first and second ranges (default to `std::identity`)
 *
 * ### Return value
 * `true` if the second range is empty or occurs as a contiguous subsequence of the first range, `false` otherwise.
 *
 * ### Complexity
 * At most `N * M` applications of `pred`, `proj1`, and `proj2` in the worst case
 * (where `N = std::ranges::distance(first1, last1)` and `M = std::ranges::distance(first2, last2)`),
 * typically linear in `N` via `raze::algorithm::search`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::search`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **`raze::options::aligned<...>`:** Supports broadcast `aligned<N>` and positional
 *   `aligned<Haystack, Needle>` (2 ranges).
 *
 * ### Notes
 * - Immediately returns `true` when the subrange is empty; otherwise delegates to `raze::algorithm::search`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/ContainsSubrange.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5};
 *     const std::vector<int> sub{2, 3, 4};
 *     const std::vector<int> miss{2, 4};
 *
 *     std::cout << std::boolalpha
 *               << raze::algorithm::contains_subrange(v, sub) << ' '
 *               << raze::algorithm::contains_subrange(v, miss) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * true false
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::search`
 * - `raze::algorithm::find_end`
 * - `raze::algorithm::contains`
 * - `raze::algorithm::starts_with`
 * - `raze::algorithm::ends_with`
*/
constexpr inline auto contains_subrange = options::function_with_traits<contains_subrange_t>;

__RAZE_ALGORITHM_NAMESPACE_END
