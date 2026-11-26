#pragma once

#include <raze/algorithm/remove/RemoveCopyIf.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/ProjectedValue.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_copy_t : Traits {
	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable OutIt,
		class Proj = std::identity, class T = projected_value_t<It, Proj>>
	constexpr raze_always_inline decltype(auto) operator()(It first, Sent last,
		OutIt out, const T& v, Proj proj = {}) const
			requires(std::indirectly_copyable<It, OutIt> &&
				std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Proj>, const T*>)
	{
		return algorithm::with_equal_to<std::iter_value_t<It>, Proj>(v,
			[&](auto&& eq) raze_always_inline_lambda {
				return remove_copy_if[Traits::traits()](std::move(first), std::move(last),
					std::move(out), std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
			});
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt,
		class Proj = std::identity, class T = projected_value_t<std::ranges::iterator_t<R>, Proj>>
	constexpr raze_always_inline decltype(auto) operator()(R&& r, OutIt out,
		const T& v, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> &&
				std::indirect_binary_predicate<std::ranges::equal_to,
					std::projected<std::ranges::iterator_t<R>, Proj>, const T*>)
	{
		return algorithm::with_equal_to<std::ranges::range_value_t<R>, Proj>(v,
			[&](auto&& eq) raze_always_inline_lambda {
				return remove_copy_if[Traits::traits()](std::forward<R>(r), std::move(out),
					std::forward<decltype(eq)>(eq), traits::fwd_fn(proj));
			});
	}
};

/**
 * @ingroup raze_algo_remove
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range to another range, omitting elements equal to a specific value.
 *
 * 1) Copies all elements from `[first, last)` to the destination range beginning at `out`,
 *    omitting those for which `bool(std::invoke(proj, *i) == v)` evaluates to `true`. Relative order is preserved.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `remove_copy(std::ranges::begin(r), std::ranges::end(r), std::move(out), v, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable OutIt,
 *           class T = std::iter_value_t<It>, class Proj = std::identity >
 *   requires(std::indirectly_copyable<It, OutIt> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Proj>, const T*>)
 * constexpr std::ranges::in_out_result<It, OutIt>
 * remove_copy( It first, Sent last, OutIt out, const T& v, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable OutIt,
 *           class T = std::ranges::range_value_t<R>, class Proj = std::identity >
 *   requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<R>, Proj>, const T*>)
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, OutIt>
 * remove_copy( R&& r, OutIt out, const T& v, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to copy from
 * - `r`             - the range of elements to copy from
 * - `out`           - the beginning of the destination range
 * - `v`             - the value of elements not to copy
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(r)`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * comparisons and applications of `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::remove_copy_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - The behavior is undefined if the source and destination ranges overlap.
 * - Implemented in terms of `raze::algorithm::remove_copy_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/remove/RemoveCopy.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> src{1, 2, 3, 2, 4, 2, 5};
 *     std::vector<int> dst(src.size());
 *
 *     auto [in, out] = raze::algorithm::remove_copy(src, dst.begin(), 2);
 *     dst.erase(out, dst.end());
 *
 *     println(dst);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 1 3 4 5
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::remove_copy_if`
 * - `raze::algorithm::remove`
 * - `raze::algorithm::copy_if`
*/
constexpr inline auto remove_copy = options::function_with_traits<remove_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END