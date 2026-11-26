#pragma once 

#include <raze/algorithm/replace/ReplaceCopyIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_copy_t : Traits {
	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent,
		std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<InIt, OutIt> operator()(
		InIt first, Sent sent, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<InIt, OutIt> && std::indirectly_writable<OutIt, const T2&> &&
				std::indirect_binary_predicate<std::ranges::equal_to, std::projected<InIt, Proj>, const T1*>)
	{
		return algorithm::with_equal_to<std::iter_value_t<InIt>, Proj>(old,
			[&](auto&& eq) raze_always_inline_lambda {
				return replace_copy_if[Traits::traits()](std::move(first), std::move(sent), std::move(out),
					std::forward<decltype(eq)>(eq), v, traits::fwd_fn(proj));
			});
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::borrowed_iterator_t<R>, OutIt> operator()(
		R&& r, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> && std::indirectly_writable<OutIt, const T2&> &&
				std::indirect_binary_predicate<std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>, const T1*>)
	{
		return algorithm::with_equal_to<std::ranges::range_value_t<R>, Proj>(old,
			[&](auto&& eq) raze_always_inline_lambda {
				return replace_copy_if[Traits::traits()](std::forward<R>(r), std::move(out),
					std::forward<decltype(eq)>(eq), v, traits::fwd_fn(proj));
			});
	}
};

/**
 * @ingroup raze_algo_replace
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range to another range, replacing all elements equal to a given value with a new value.
 *
 * 1) Copies elements from `[first, sent)` to the destination range beginning at `out`,
 *    replacing each element with `v` if `bool(std::invoke(proj, *i) == old)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `replace_copy(std::ranges::begin(r), std::ranges::end(r), std::move(out), old, v, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent,
 *           std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_copyable<InIt, OutIt> &&
 *            std::indirectly_writable<OutIt, const T2&> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to, std::projected<InIt, Proj>, const T1*>)
 * constexpr std::ranges::unary_transform_result<InIt, OutIt>
 * replace_copy( InIt first, Sent sent, OutIt out,
 *               const T1& old, const T2& v, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable OutIt,
 *           class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> &&
 *            std::indirectly_writable<OutIt, const T2&> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Proj>, const T1*>)
 * constexpr std::ranges::unary_transform_result<std::ranges::borrowed_iterator_t<R>, OutIt>
 * replace_copy( R&& r, OutIt out,
 *               const T1& old, const T2& v, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to copy from
 * - `r`             - the range of elements to copy from
 * - `out`           - the beginning of the destination range
 * - `old`           - the value of elements to replace
 * - `v`             - the replacement value
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::unary_transform_result` containing:
 * - `in`: an iterator comparing equal to `sent` (or `std::ranges::end(r)`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`)
 * comparisons and applications of `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::replace_copy_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `replace_copy_strategy` (`strategy_mode::autovec` on GCC, Clang, and ICPX).
 * - **`raze::options::aligned<...>`:** Supports broadcast `aligned<N>` and positional `aligned<In, Out>` (2 ranges).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::replace_copy_if` (see @ref autovec_behavior).
 *
 * ### Notes
 * - The behavior is undefined if the source and destination ranges overlap.
 * - Implemented in terms of `raze::algorithm::replace_copy_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/replace/ReplaceCopy.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> src{5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
 *     std::vector<int> dst(src.size());
 *
 *     // Copy elements while replacing all occurrences of 8 with 88
 *     raze::algorithm::replace_copy(src, dst.begin(), 8, 88);
 *
 *     println(src);
 *     println(dst);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 5 7 4 2 8 6 1 9 0 3
 * 5 7 4 2 88 6 1 9 0 3
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::replace_copy_if`
 * - `raze::algorithm::replace`
 * - `raze::algorithm::replace_if`
*/
constexpr inline auto replace_copy = options::function_with_traits<replace_copy_t>[options::unroll<4>][replace_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
