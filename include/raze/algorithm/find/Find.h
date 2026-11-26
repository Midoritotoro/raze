#pragma once

#include <raze/algorithm/find/FindIf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline It operator()(It first, Sent last, const T& value, Projection proj = {}) const
        requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::iter_value_t<It>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return find_if[Traits::traits()](std::move(first), std::move(last),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }

    template <std::ranges::input_range R, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
    operator()(R&& r, const T& value, Projection proj = {}) const requires(std::indirect_binary_predicate<
        std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::ranges::range_value_t<R>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return find_if[Traits::traits()](std::forward<R>(r),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the first element in a range equal to a given value.
 *
 * 1) Searches `[first, last)` for the first iterator `i` such that
 *    `bool(std::invoke(proj, *i) == value)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `find(std::ranges::begin(r), std::ranges::end(r), value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
 * constexpr It find( It first, Sent last, const T& value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * find( R&& r, const T& value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `value`         - the value to search for
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator to the first element comparing equal to `value`, or an iterator comparing equal to
 * `last` (or `std::ranges::end(r)`) if no such element is found.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` comparisons and applications of `proj` (short-circuiting on the first match).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::find_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::find_if` (see @ref autovec_behavior).
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::find_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/Find.h>
 *
 * int main() {
 *     const std::vector<int> v{10, 20, 30, 40, 50};
 *
 *     if (auto it = raze::algorithm::find(v, 30); it != v.end())
 *         std::cout << "Found 30 at index " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Found 30 at index 2
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find_if`
 * - `raze::algorithm::find_if_not`
 * - `raze::algorithm::find_last`
 * - `raze::algorithm::contains`
*/
constexpr inline auto find = options::function_with_traits<find_t>;

__RAZE_ALGORITHM_NAMESPACE_END