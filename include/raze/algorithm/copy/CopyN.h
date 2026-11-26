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
 * @ingroup raze_algo_copy
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies a specified number of elements from a range to another location.
 *
 * Copies `n` elements from the range beginning at `first` to the destination range beginning at `out`,
 * if `n > 0`. Does nothing and returns `{first, out}` if `n <= 0`.
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
 * - `n`     - the number of elements to copy
 * - `out`   - the beginning of the destination range (must be large enough to hold `max(0, n)` elements)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator pointing to `std::ranges::next(first, n)` (or `first` if `n <= 0`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `max(0, n)` assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::copy`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`.
 *
 * ### Notes
 * - The behavior is undefined if `[first, first + n)` and `[out, out + n)` overlap.
 * - Implemented by delegating to `raze::algorithm::copy` over `std::counted_iterator(first, n)`
 *   and `std::default_sentinel`.
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
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::copy`
 * - `raze::algorithm::copy_backward`
 * - `raze::algorithm::copy_if`
*/
constexpr inline auto copy_n = options::function_with_traits<copy_n_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
