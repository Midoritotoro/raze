#pragma once

#include <raze/algorithm/fold/FoldLeftWithIter.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fold_left_t : Traits {
	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class T = std::iter_value_t<It>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It first, Sent last, T init, F f) const
		requires(detail::indirectly_binary_left_foldable<F, T, It>)
	{
		return fold_left_with_iter[Traits::traits()](
			std::move(first), std::move(last), std::move(init), std::move(f)).value;
	}

	template <std::ranges::input_range R,
		class T = std::ranges::range_value_t<R>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R&& r, T init, F f) const
		requires(detail::indirectly_binary_left_foldable<F, T, std::ranges::iterator_t<R>>)
	{
		return fold_left_with_iter[Traits::traits()](
			std::forward<R>(r), std::move(init), std::move(f)).value;
	}
};

/**
 * @ingroup raze_algo_fold
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Left-folds the elements of a range starting from an initial value (C++23).
 *
 * 1) Computes `f(...f(f(init, x_0), x_1)..., x_{n-1})` across `[first, last)` in order.
 *
 * 2) Same as (1), but uses `r` as the source range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>, class F >
 * constexpr auto
 * fold_left( It first, Sent last, T init, F f );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T = std::ranges::range_value_t<R>, class F >
 * constexpr auto
 * fold_left( R&& r, T init, F f );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - input range to fold
 * - `r`             - input range to fold
 * - `init`          - initial accumulator value (supports braced-init lists via C++26 P2248)
 * - `f`             - binary fold callable `f(accum, elem)`
 *
 * ### Return value
 * The final accumulated value of decayed invoke-result type `U`.
 *
 * ### Complexity
 * Linear: exactly `N` invocations of `f`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <functional>
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fold/FoldLeft.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4};
 *     std::cout << raze::algorithm::fold_left(v, 10, std::plus<>{}) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fold_left_with_iter`
 * - `raze::algorithm::fold_left_first`
 * - `raze::algorithm::fold_right`
 */
constexpr inline auto fold_left = options::function_with_traits<fold_left_t>;

__RAZE_ALGORITHM_NAMESPACE_END

