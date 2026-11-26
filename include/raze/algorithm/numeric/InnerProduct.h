#pragma once

#include <raze/algorithm/numeric/TransformReduce.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct inner_product_t : Traits {
	template <std::input_iterator It1, std::sentinel_for<It1> Sent1, std::input_iterator It2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It1 first1, Sent1 last1, It2 first2, T init,
		BinaryOp1 op1 = {}, BinaryOp2 op2 = {}) const
			requires(!std::sentinel_for<T, It2> &&
				std::invocable<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>> &&
				std::invocable<BinaryOp1&, T,
					std::invoke_result_t<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>>>)
	{
		for (; first1 != last1; ++first1, ++first2)
			init = std::invoke(op1, std::move(init), std::invoke(op2, *first1, *first2));
		return init;
	}

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::input_iterator It2, std::sentinel_for<It2> Sent2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2, T init,
		BinaryOp1 op1 = {}, BinaryOp2 op2 = {}) const
			requires(std::invocable<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>> &&
				std::invocable<BinaryOp1&, T,
					std::invoke_result_t<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>>>)
	{
		for (; first1 != last1 && first2 != last2; ++first1, ++first2)
			init = std::invoke(op1, std::move(init), std::invoke(op2, *first1, *first2));
		return init;
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(R1&& r1, R2&& r2, T init, BinaryOp1 op1 = {}, BinaryOp2 op2 = {}) const
		requires(std::invocable<BinaryOp2&, std::ranges::range_reference_t<R1>, std::ranges::range_reference_t<R2>> &&
			std::invocable<BinaryOp1&, T,
				std::invoke_result_t<BinaryOp2&, std::ranges::range_reference_t<R1>, std::ranges::range_reference_t<R2>>>)
	{
		return (*this)(std::ranges::begin(r1), std::ranges::end(r1),
			std::ranges::begin(r2), std::ranges::end(r2),
			std::move(init), traits::fwd_fn(op1), traits::fwd_fn(op2));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Computes the sequential inner product (sum of element-wise products) of two ranges.
 *
 * 1) Evaluates `init = std::invoke(op1, std::move(init), std::invoke(op2, *i1, *i2))` left-to-right
 *    for each element of `[first1, last1)` paired with elements starting at `first2`.
 *
 * 2) Same as (1), bounded by both `[first1, last1)` and `[first2, last2)`.
 *
 * 3) Same as (2), using ranges `r1` and `r2`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Unbounded second sequence
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1, std::input_iterator It2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T inner_product( It1 first1, Sent1 last1, It2 first2, T init,
 *                            BinaryOp1 op1 = {}, BinaryOp2 op2 = {} );
 *
 * // (2) Bounded iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T inner_product( It1 first1, Sent1 last1, It2 first2, Sent2 last2, T init,
 *                            BinaryOp1 op1 = {}, BinaryOp2 op2 = {} );
 *
 * // (3) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T inner_product( R1&& r1, R2&& r2, T init,
 *                            BinaryOp1 op1 = {}, BinaryOp2 op2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1`, `r1` - first input range
 * - `first2`, `last2`, `r2` - second input range
 * - `init`                  - initial accumulator value
 * - `op1`                   - outer reduction operation (defaults to `std::plus<>`)
 * - `op2`                   - element-wise combining operation (defaults to `std::multiplies<>`)
 *
 * ### Return value
 * The accumulated inner product of type `T`.
 *
 * ### Complexity
 * Linear: `min(N1, N2)` applications of `op1` and `op2`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/InnerProduct.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3}, b{4, 5, 6};
 *     std::cout << raze::algorithm::inner_product(a, b, 0) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::transform_reduce`
 * - `raze::algorithm::accumulate`
 */
constexpr inline auto inner_product = options::function_with_traits<inner_product_t>;

__RAZE_ALGORITHM_NAMESPACE_END

