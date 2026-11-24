#pragma once

#include <raze/algorithm/copy/Copy.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_n_t : Traits {
	template <std::input_iterator InIt,  std::weakly_incrementable OutIt>
		constexpr raze_always_inline std::ranges::in_out_result<InIt, OutIt> operator()(InIt first,
			std::iter_difference_t<InIt> n, OutIt out) const requires(std::indirectly_copyable<InIt, OutIt>)
	{
		if (n <= 0) return { std::move(first), std::move(out) };
		auto [in_last, out_last] = copy[Traits::traits()](std::counted_iterator<InIt>(
			std::move(first), n), std::default_sentinel, std::move(out));
		return { std::move(in_last).base(), std::move(out_last) };
	}
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies a specified number of elements from a range to another location.
 *
 * Copies `n` elements from the range beginning at `first` to the destination range beginning at `out`,
 * if `n > 0`. Does nothing if `n == 0` (the behavior is undefined if `n < 0`).
 *
 * ### Declarations
 * ```cpp
 * template< std::input_iterator InIt, std::weakly_incrementable OutIt >
 *   requires(std::indirectly_copyable<InIt, OutIt>)
 * constexpr std::ranges::in_out_result<InIt, OutIt>
 * copy_n( InIt first, std::iter_difference_t<InIt> n, OutIt out );
 * ```
 *
 * ### Parameters
 * - `first` - the beginning of the source range
 * - `n`     - the number of elements to copy (must satisfy `n >= 0`)
 * - `out`   - the beginning of the destination range (must be large enough to hold `n` elements)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator pointing to `std::ranges::next(first, n)` (or `first` if `n == 0`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `n` assignments (or `0` if `n == 0`).
 *
 * ### Decorators and Options
 * The algorithm object is configured with `raze::options::unroll<4>` by default and supports
 * compile-time modifiers via `operator[]` (forwarded to `raze::algorithm::copy`):
 * ```cpp
 * copy_n[raze::options::fscalar](...);
 * copy_n[raze::options::unroll<2>](...);
 * copy_n[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - The behavior is undefined if the source range `[first, first + n)` and destination range
 *   `[out, out + n)` overlap.
 * - Implemented by delegating to `raze::algorithm::copy` using `std::counted_iterator` and
 *   `std::default_sentinel`, inheriting its execution paths (`memcpy` for identical trivially copyable
 *   contiguous types, explicit SIMD `raze::vx::convert` for supported arithmetic conversions, and
 *   scalar fallback otherwise).
 * - In a constant-evaluated context (`if consteval`) or when `fscalar` is applied, the algorithm
 *   executes via the scalar fallback path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/copy/CopyN.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> src{10, 20, 30, 40, 50};
 *     std::vector<int> dst(3);
 *
 *     auto [in, out] = raze::algorithm::copy_n(src.begin(), 3, dst.begin());
 *     println(dst);
 *     std::cout << "Next unread element: " << *in << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 10 20 30
 * Next unread element: 40
 * ```
*/
constexpr inline auto copy_n = options::function_with_traits<copy_n_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
