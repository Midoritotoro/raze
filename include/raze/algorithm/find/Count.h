#pragma once

#include <raze/algorithm/find/CountIf.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/ProjectedValue.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct count_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Projection = std::identity, class T = projected_value_t<It, Projection>>
    raze_nodiscard constexpr raze_always_inline std::iter_difference_t<It>
        operator()(It first, Sent last, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::iter_value_t<It>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return count_if[Traits::traits()](std::move(first), std::move(last),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }

    template <std::ranges::input_range R,
        class Projection = std::identity, class T = projected_value_t<std::ranges::iterator_t<R>, Projection>>
    raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<R>
        operator()(R&& r, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to,
                std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return algorithm::with_equal_to<std::ranges::range_value_t<R>, Projection>(value,
            [&](auto&& eq) raze_always_inline_lambda {
                return count_if[Traits::traits()](std::forward<R>(r),
                    std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
            });
    }
};

/**
 * @ingroup raze_algo_count
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Counts the elements in a range that are equal to a given value.
 *
 * 1) Counts all elements in `[first, last)` for which `bool(std::invoke(proj, *i) == value)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `count(std::ranges::begin(r), std::ranges::end(r), value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
 * constexpr std::iter_difference_t<It>
 * count( It first, Sent last, const T& value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T, class Projection = std::identity >
 *   requires(std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
 * constexpr std::ranges::range_difference_t<R>
 * count( R&& r, const T& value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `value`         - the value to count
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * The number of elements comparing equal to `value`.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * comparisons and applications of `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::count_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`.
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::count_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/Count.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 2, 4, 2, 5};
 *
 *     std::cout << "Count of 2: " << raze::algorithm::count(v, 2) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Count of 2: 3
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::count_if`
 * - `raze::algorithm::count_if_not`
 * - `raze::algorithm::contains`
 * - `raze::algorithm::find`
*/
constexpr inline auto count = options::function_with_traits<count_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END