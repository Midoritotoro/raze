#pragma once

#include <raze/algorithm/find/FindIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_if_not_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline It operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return find_if[Traits::traits()](std::move(first), std::move(last), make_not_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
    operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return find_if[Traits::traits()](std::forward<R>(r), make_not_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the first element in a range that does not satisfy a unary predicate.
 *
 * 1) Searches `[first, last)` for the first iterator `i` such that
 *    `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `false`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `find_if_not(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr It find_if_not( It first, Sent last, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * find_if_not( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `pred`          - unary predicate returning `false` for the target element
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator to the first element not satisfying `pred`, or an iterator comparing equal to
 * `last` (or `std::ranges::end(r)`) if every element satisfies `pred`.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` applications of `pred` and `proj` (short-circuiting on the first match).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::find_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::find_if` (see @ref autovec_behavior).
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::find_if` with a negated predicate (`make_not_fn`).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/FindIfNot.h>
 *
 * int main() {
 *     const std::vector<int> v{2, 4, 6, 7, 8};
 *
 *     if (auto it = raze::algorithm::find_if_not(v, [](auto x) { return (x & 1) == 0; }); it != v.end())
 *         std::cout << "First odd element: " << *it << " at index " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * First odd element: 7 at index 3
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find_if`
 * - `raze::algorithm::find`
 * - `raze::algorithm::find_last_if_not`
 * - `raze::algorithm::all_of`
*/
constexpr inline auto find_if_not = options::function_with_traits<find_if_not_t>;

__RAZE_ALGORITHM_NAMESPACE_END