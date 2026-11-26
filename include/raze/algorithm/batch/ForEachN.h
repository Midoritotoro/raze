#pragma once

#include <raze/algorithm/batch/ForEach.h>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct for_each_n_t : Traits {
	template <std::input_iterator InIt, class F, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::in_fun_result<InIt, F> operator()(
		InIt first, std::iter_difference_t<InIt> n, F f, Proj proj = {}) const
			requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
	{
		auto r = for_each[Traits::traits()](std::counted_iterator<InIt>(std::move(first), n),
			std::default_sentinel, traits::fwd_fn(f), traits::fwd_fn(proj));
		return { r.in.base(), std::move(r.fun)};
	}
};

/**
 * @ingroup raze_algo_batch
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Applies a function object to the first `n` elements of a sequence.
 *
 * Applies the unary function `f` to the result of dereferencing every iterator in the range
 * `[first, first + n)`, projected by `proj`, if `n > 0`. Does nothing if `n == 0`
 * (the behavior is undefined if `n < 0`).
 *
 * ### Declarations
 * ```cpp
 * template< std::input_iterator InIt, class F, class Proj = std::identity >
 *   requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
 * constexpr std::ranges::in_fun_result<InIt, F>
 * for_each_n( InIt first, std::iter_difference_t<InIt> n, F f, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first` - the beginning of the range of elements to iterate over
 * - `n`     - the number of elements to process (must satisfy `n >= 0`)
 * - `f`     - unary function object to apply to each projected element
 * - `proj`  - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::in_fun_result` containing:
 * - `in`: an iterator pointing past the last processed element (`std::ranges::next(first, n)` if `n > 0`, otherwise `first`).
 * - `fun`: the function object `f`.
 *
 * ### Complexity
 * Linear in `n` (`0` if `n == 0`): `n` applications of `f` and `proj` in the scalar and `autovec` paths,
 * or one vector application per SIMD chunk (including the masked tail) in the explicit SIMD path.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::for_each`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `for_each_strategy` (`strategy_mode::autovec` on GCC, Clang, and ICPX).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range starting at `first` (1 range).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::for_each` (see @ref autovec_behavior).
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::for_each` over `std::counted_iterator(first, n)`
 *   and `std::default_sentinel`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/batch/ForEachN.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
 *
 *     // Vectorized in-place modification of the first 4 elements (`auto&` enables SIMD / autovec)
 *     raze::algorithm::for_each_n(v.begin(), 4, [](auto& x) { x *= 10; });
 *     println(v);
 *
 *     // Strict scalar in-order traversal of the first 3 elements
 *     int sum = 0;
 *     raze::algorithm::for_each_n[raze::options::fscalar](v.begin(), 3, [&sum](const auto& x) { sum += x; });
 *     std::cout << "Sum of first 3: " << sum << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 10 20 30 40 5 6 7
 * Sum of first 3: 60
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::for_each`
*/
constexpr inline auto for_each_n = options::function_with_traits<for_each_n_t>[options::unroll<4>][for_each_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
