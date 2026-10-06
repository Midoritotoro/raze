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
 * @brief Applies a function object to the first n elements of a sequence, in order.
 *
 * Applies the unary function `f` to the result of dereferencing every iterator in the range
 * `[first, first + n)`, projected by `proj`. If `n <= 0`, the algorithm has no effect.
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
 * - `n`     - the number of elements to process
 * - `f`     - unary function object to apply to projected elements
 * - `proj`  - projection to apply to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::in_fun_result` containing:
 * - `in`: an iterator pointing past the last processed element (`first + n` if `n > 0`, otherwise `first`).
 * - `fun`: the function object `f` passed as an argument.
 *
 * ### Complexity
 * Linear: exactly `max(0, n)` invocations of `f` and applications of `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * for_each_n[raze::options::fscalar](...);
 * for_each_n[raze::options::unroll<2>](...);
 * for_each_n[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
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
 *     // In-place modification of the first 4 elements
 *     raze::algorithm::for_each_n(v.begin(), 4, [](int& x) { x *= 10; });
 *     println(v);
 *
 *     // Non-modifying traversal of the first 3 elements with side effects
 *     int sum = 0;
 *     raze::algorithm::for_each_n(v.begin(), 3, [&sum](int x) { sum += x; });
 *     std::cout << "Sum of first 3: " << sum << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 10 20 30 40 5 6 7 
 * Sum of first 3: 60
 * ```
 */
constexpr inline auto for_each_n = options::function_with_traits<for_each_n_t>[options::unroll<4>][for_each_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
