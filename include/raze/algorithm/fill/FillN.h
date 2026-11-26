#pragma once

#include <raze/algorithm/fill/Fill.h>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fill_n_t : Traits {
	template <std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>,
		class T = iter_value_or_void_t<OutIt>>
	constexpr raze_always_inline OutIt operator()(OutIt first, Size n, const T& value) const
		requires(std::output_iterator<OutIt, const T&>)
	{
		if (n <= 0) return first;
		return fill[Traits::traits()](std::counted_iterator<OutIt>(
			std::move(first), static_cast<std::iter_difference_t<OutIt>>(n)), std::default_sentinel, value).base();
	}
};

/**
 * @ingroup raze_algo_fill
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Assigns the given value to the first `n` elements of a sequence.
 *
 * 1) Assigns `value` to every element in the range `[first, first + n)`.
 *    If `n <= 0`, the algorithm has no effect and returns `first`.
 *
 * 2) Same as (1), for pure output iterators that are not `std::indirectly_readable`
 *    (such as `std::back_insert_iterator`).
 *
 * ### Declarations
 * ```cpp
 * // (1) Readable iterator overload (defaulted value type)
 * template< std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>,
 *           class T = std::iter_value_t<OutIt> >
 *   requires(std::indirectly_readable<OutIt> && std::output_iterator<OutIt, const T&>)
 * constexpr OutIt fill_n( OutIt first, Size n, const std::type_identity_t<T>& value );
 *
 * // (2) Pure output iterator overload
 * template< std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>, class T >
 *   requires(!std::indirectly_readable<OutIt> && std::output_iterator<OutIt, const T&>)
 * constexpr OutIt fill_n( OutIt first, Size n, const T& value );
 * ```
 *
 * ### Parameters
 * - `first` - the beginning of the range of elements to modify
 * - `n`     - the number of elements to assign
 * - `value` - the value to be assigned
 *
 * ### Return value
 * An iterator pointing past the last assigned element (`std::ranges::next(first, n)` if `n > 0`, otherwise `first`).
 *
 * ### Complexity
 * Linear: exactly `max(0, n)` assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::fill`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `fill_strategy` (`strategy_mode::manual` on GCC,
 *   `strategy_mode::autovec` on Clang and ICPX).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range starting at `first` (1 range).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::fill` (see @ref autovec_behavior).
 *
 * ### Notes
 * - Implemented by delegating to `raze::algorithm::fill` over `std::counted_iterator(first, n)`
 *   and `std::default_sentinel`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fill/FillN.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v(8, 0);
 *
 *     // Fill the first 5 elements with 42
 *     raze::algorithm::fill_n(v.begin(), 5, 42);
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 42 42 42 42 42 0 0 0
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fill`
*/
constexpr inline auto fill_n = options::function_with_traits<fill_n_t>[options::unroll<4>][fill_strategy];

__RAZE_ALGORITHM_NAMESPACE_END