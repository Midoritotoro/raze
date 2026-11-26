#pragma once

#include <raze/algorithm/fold/FoldLeftFirstWithIter.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fold_left_first_t : Traits {
	template <std::input_iterator It, std::sentinel_for<It> Sent, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It first, Sent last, F f) const
		requires(std::constructible_from<std::iter_value_t<It>, std::iter_reference_t<It>> &&
			detail::indirectly_binary_left_foldable<F, std::iter_value_t<It>, It>)
	{
		return fold_left_first_with_iter[Traits::traits()](
			std::move(first), std::move(last), std::move(f)).value;
	}

	template <std::ranges::input_range R, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R&& r, F f) const
		requires(std::constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>> &&
			detail::indirectly_binary_left_foldable<F, std::ranges::range_value_t<R>, std::ranges::iterator_t<R>>)
	{
		return fold_left_first_with_iter[Traits::traits()](
			std::forward<R>(r), std::move(f)).value;
	}
};

/**
 * @ingroup raze_algo_fold
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Left-folds a range using its first element as the initial value (C++23).
 *
 * 1) Returns `std::optional` containing `f(...f(f(x_0, x_1), x_2)..., x_{n-1})`, or `std::nullopt`
 *    if `[first, last)` is empty.
 *
 * 2) Same as (1), but uses `r` as the source range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent, class F >
 * constexpr std::optional<U>
 * fold_left_first( It first, Sent last, F f );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, class F >
 * constexpr std::optional<U>
 * fold_left_first( R&& r, F f );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - input range to fold
 * - `r`             - input range to fold
 * - `f`             - binary fold callable `f(accum, elem)`
 *
 * ### Return value
 * `std::optional<U>` holding the folded value, or `std::nullopt` if the range is empty.
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` invocations of `f`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <functional>
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fold/FoldLeftFirst.h>
 *
 * int main() {
 *     const std::vector<int> v{10, 2, 3};
 *     if (auto r = raze::algorithm::fold_left_first(v, std::minus<>{}))
 *         std::cout << *r << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fold_left_first_with_iter`
 * - `raze::algorithm::fold_left`
 * - `raze::algorithm::fold_right_last`
 */
constexpr inline auto fold_left_first = options::function_with_traits<fold_left_first_t>;

__RAZE_ALGORITHM_NAMESPACE_END

