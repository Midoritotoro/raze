#pragma once

#include <raze/algorithm/fill/Fill.h>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fill_n_t : Traits {
	template <std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>,
		class T = std::iter_value_t<OutIt>>
	constexpr raze_always_inline OutIt operator()(OutIt first, Size n,
		const std::type_identity_t<T>& value) const requires(std::output_iterator<OutIt, const T&>)
	{
		if (n <= 0) return first;
		return fill[Traits::traits()](std::counted_iterator<OutIt>(
			std::move(first), n), std::default_sentinel, value).base();
	}
};

/**
 * @brief Assigns the given value to the first n elements of a sequence.
 *
 * Assigns `value` to every element in the range `[first, first + n)`.
 * If `n <= 0`, the algorithm has no effect and returns `first`.
 *
 * ### Declarations
 * ```cpp
 * template< std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>,
 *           class T = std::iter_value_t<OutIt> >
 *   requires(std::output_iterator<OutIt, const T&>)
 * constexpr OutIt fill_n( OutIt first, Size n, const std::type_identity_t<T>& value );
 * ```
 *
 * ### Parameters
 * - `first` - the beginning of the range of elements to modify
 * - `n`     - the number of elements to assign
 * - `value` - the value to be assigned
 *
 * ### Return value
 * An iterator pointing past the last assigned element (`first + n` if `n > 0`, otherwise `first`).
 *
 * ### Complexity
 * Linear: exactly `max(0, n)` assignments.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * fill_n[raze::options::fscalar](...);
 * fill_n[raze::options::unroll<2>](...);
 * fill_n[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
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
 */
constexpr inline auto fill_n = options::function_with_traits<fill_n_t>[options::unroll<4>][fill_strategy];

__RAZE_ALGORITHM_NAMESPACE_END