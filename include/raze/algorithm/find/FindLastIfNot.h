#pragma once

#include <raze/algorithm/find/FindLastIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_last_if_not_t : Traits {
    template <std::forward_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return find_last_if[Traits::traits()](std::move(first), std::move(last), make_not_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return find_last_if[Traits::traits()](std::forward<R>(r), make_not_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the last element in a range that does not satisfy a unary predicate.
 *
 * 1) Searches `[first, last)` for the last iterator `i` such that
 *    `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `false`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `find_last_if_not(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr std::ranges::subrange<It>
 * find_last_if_not( It first, Sent last, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * find_last_if_not( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `pred`          - unary predicate returning `false` for the target element
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `[found, end)` spanning from the last non-matching element to the end of the range,
 * or an empty subrange `{end, end}` if every element satisfies `pred`.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` applications of `pred` and `proj` (short-circuiting from the end on random-access/contiguous ranges).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::find_last_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::find_last_if` with a negated predicate (`make_not_fn`).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/FindLastIfNot.h>
 *
 * int main() {
 *     const std::vector<int> v{2, 3, 4, 7, 6, 8};
 *
 *     auto tail = raze::algorithm::find_last_if_not(v, [](auto x) { return (x & 1) == 0; });
 *     if (!tail.empty())
 *         std::cout << "Last odd element: " << *tail.begin()
 *                   << " at index " << (tail.begin() - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Last odd element: 7 at index 3
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find_last_if`
 * - `raze::algorithm::find_last`
 * - `raze::algorithm::find_if_not`
 * - `raze::algorithm::find_end`
*/
constexpr inline auto find_last_if_not = options::function_with_traits<find_last_if_not_t>;

__RAZE_ALGORITHM_NAMESPACE_END