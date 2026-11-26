#pragma once

#include <raze/algorithm/find/FindLastIf.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/ProjectedValue.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_last_t : Traits {
    template <std::forward_iterator It, std::sentinel_for<It> Sent,
        class Projection = std::identity, class T = projected_value_t<It, Projection>>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent last, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::iter_value_t<It>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return find_last_if[Traits::traits()](std::move(first), std::move(last),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }

    template <std::ranges::forward_range R,
        class Projection = std::identity, class T = projected_value_t<std::ranges::iterator_t<R>, Projection>>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to, 
                std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::ranges::range_value_t<R>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return find_last_if[Traits::traits()](std::forward<R>(r),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the last element in a range equal to a given value.
 *
 * 1) Searches `[first, last)` for the last iterator `i` such that
 *    `bool(std::invoke(proj, *i) == value)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `find_last(std::ranges::begin(r), std::ranges::end(r), value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
 * constexpr std::ranges::subrange<It>
 * find_last( It first, Sent last, const T& value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * find_last( R&& r, const T& value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `value`         - the value to search for
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `[found, end)` spanning from the last matching element to the end of the range,
 * or an empty subrange `{end, end}` if no matching element is found.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` comparisons and applications of `proj` (short-circuiting from the end on random-access/contiguous ranges).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::find_last_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::find_last_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/FindLast.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 2, 4, 2, 5};
 *
 *     auto tail = raze::algorithm::find_last(v, 2);
 *     if (!tail.empty())
 *         std::cout << "Last 2 at index " << (tail.begin() - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Last 2 at index 5
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find_last_if`
 * - `raze::algorithm::find_last_if_not`
 * - `raze::algorithm::find`
 * - `raze::algorithm::find_end`
*/
constexpr inline auto find_last = options::function_with_traits<find_last_t>;

__RAZE_ALGORITHM_NAMESPACE_END