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
 * @ingroup raze_algo
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
 * Linear in `n` (`0` if `n == 0`): performs `n` applications of `f` and `proj` in the scalar
 * and auto-vectorized paths, or one vector application per SIMD chunk (including the masked tail)
 * in the explicit SIMD path.
 *
 * ### Decorators and Options
 * The algorithm object is configured with `raze::options::unroll<4>` by default (and
 * `strategy_mode::autovec` on GCC, Clang, and ICPX) and supports compile-time modifiers via
 * `operator[]` (forwarded to `raze::algorithm::for_each`):
 * ```cpp
 * for_each_n[raze::options::fscalar](...);
 * for_each_n[raze::options::unroll<2>](...);
 * for_each_n[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::for_each` using `std::counted_iterator` and
 *   `std::default_sentinel`, inheriting its execution paths (`autovec` on GCC/Clang/ICPX,
 *   explicit SIMD with masked tail handling on MSVC, and scalar fallback otherwise).
 * - **Execution order & vectorization**:
 *   - When `f` is vectorizable (e.g., accepts `auto&`), elements may be processed in an unsequenced
 *     (`unseq`) / vectorized manner without strict element-by-element ordering; `f` must not rely on
 *     loop-carried dependencies or cross-iteration mutable state.
 *   - Strict sequential in-order execution is guaranteed when `f` takes an explicit scalar parameter
 *     type (e.g., `[](int x)` or `[](int& x)`), when `raze::options::fscalar` is specified, for
 *     non-contiguous iterators or non-`std::identity` projections, or in constant-evaluated contexts
 *     (`if consteval`).
 *   - See @ref vectorization_semantics for full details and caveats with generic (`auto`) lambdas.
 * - The input range must be modifiable (`modifiable_source`).
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
 *     // Guaranteed sequential in-order traversal of the first 3 elements (explicit scalar parameter)
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
