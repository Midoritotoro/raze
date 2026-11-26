#pragma once

#include <raze/algorithm/find/AnyOf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct contains_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It first, Sent last, const T& value, Projection proj = {}) const
        requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::iter_value_t<It>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return any_of[Traits::traits()](std::move(first), std::move(last),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }

    template <std::ranges::input_range R, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R&& r, const T& value, 
        Projection proj = {}) const requires(std::indirect_binary_predicate<std::ranges::equal_to,
            std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::ranges::range_value_t<R>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return any_of[Traits::traits()](std::forward<R>(r),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }
};

/**
 * @ingroup raze_algo_subrange
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether a range contains an element equal to a given value.
 *
 * 1) Returns `true` if there is at least one iterator `i` in `[first, last)` such that
 *    `bool(std::invoke(proj, *i) == value)` evaluates to `true`, and `false` otherwise.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `contains(std::ranges::begin(r), std::ranges::end(r), value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
 * constexpr bool contains( It first, Sent last, const T& value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
 * constexpr bool contains( R&& r, const T& value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `value`         - the value to search for
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * `true` if an element comparing equal to `value` is found, `false` otherwise.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` comparisons and applications of `proj` (short-circuiting on the first match).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::any_of`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::any_of` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/Contains.h>
 *
 * int main() {
 *     const std::vector<int> v{10, 20, 30, 40, 50};
 *
 *     std::cout << std::boolalpha
 *               << raze::algorithm::contains(v, 30) << ' '
 *               << raze::algorithm::contains(v, 99) << '\n';
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
 * - `raze::algorithm::contains_subrange`
 * - `raze::algorithm::any_of`
 * - `raze::algorithm::find`
 * - `raze::algorithm::count`
*/
constexpr inline auto contains = options::function_with_traits<contains_t>;

__RAZE_ALGORITHM_NAMESPACE_END