#pragma once

#include <raze/algorithm/find/CountIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct count_if_not_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::iter_difference_t<It>
        operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return count_if[Traits::traits()](std::move(first), std::move(last),
            make_not_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<R>
        operator()(R&& r, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return count_if[Traits::traits()](std::forward<R>(r), make_not_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_count
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Counts the elements in a range that do not satisfy a unary predicate.
 *
 * 1) Counts all elements in `[first, last)` for which `bool(std::invoke(pred, std::invoke(proj, *i)))`
 *    evaluates to `false`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `count_if_not(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr std::iter_difference_t<It>
 * count_if_not( It first, Sent last, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::range_difference_t<R>
 * count_if_not( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `pred`          - unary predicate returning `false` for elements to count
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * The number of elements for which `pred` evaluates to `false`.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * applications of `pred` and `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::count_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`.
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::count_if` with a negated predicate (`make_not_fn`).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/CountIfNot.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
 *
 *     const auto odds = raze::algorithm::count_if_not(v, [](auto x) { return (x & 1) == 0; });
 *     std::cout << "Odd count: " << odds << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Odd count: 4
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::count_if`
 * - `raze::algorithm::count`
 * - `raze::algorithm::find_if_not`
 * - `raze::algorithm::none_of`
*/
constexpr inline auto count_if_not = options::function_with_traits<count_if_not_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
