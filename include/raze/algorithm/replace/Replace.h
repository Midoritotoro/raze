#pragma once 

#include <raze/algorithm/replace/ReplaceIf.h>
#include <src/raze/algorithm/ProjectedValue.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_t : Traits {
	template <std::input_iterator Iter, std::sentinel_for<Iter> Sent,
		class Proj = std::identity, class T1 = projected_value_t<Iter, Proj>, class T2 = std::iter_value_t<Iter>>
	constexpr raze_always_inline void operator()(Iter first, Sent last,
		const T1& old_value, const T2& new_value, Proj proj = {}) const
			requires(std::indirectly_writable<Iter, const T2&> &&
				std::indirect_binary_predicate<std::ranges::equal_to, std::projected<Iter, Proj>, const T1*>)
	{
		algorithm::with_equal_to<std::iter_value_t<Iter>, Proj>(old_value,
			[&](auto&& eq) raze_always_inline_lambda {
				replace_if[Traits::traits()](std::move(first), std::move(last),
					std::forward<decltype(eq)>(eq), new_value, traits::fwd_fn(proj));
			});
	}

	template <std::ranges::input_range Range,
		class Proj = std::identity, class T1 = projected_value_t<std::ranges::iterator_t<Range>, Proj>,
		class T2 = std::ranges::range_value_t<Range>>
	constexpr raze_always_inline void operator()(Range&& r, const T1& old_value,
		const T2& new_value, Proj proj = {}) const
			requires(std::indirectly_writable<std::ranges::iterator_t<Range>, const T2&> &&
				std::indirect_binary_predicate<std::ranges::equal_to, std::projected<std::ranges::iterator_t<Range>, Proj>, const T1*>)
	{
		algorithm::with_equal_to<std::ranges::range_value_t<Range>, Proj>(old_value,
			[&](auto&& eq) raze_always_inline_lambda {
				replace_if[Traits::traits()](std::forward<Range>(r),
					std::forward<decltype(eq)>(eq), new_value, traits::fwd_fn(proj));
			});
	}
};

/**
 * @ingroup raze_algo_replace
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Replaces all elements equal to a given value in a range with a new value.
 *
 * 1) Replaces each element `*i` in `[first, last)` with `new_value` if
 *    `bool(std::invoke(proj, *i) == old_value)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `replace(std::ranges::begin(r), std::ranges::end(r), old_value, new_value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator Iter, std::sentinel_for<Iter> Sent,
 *           class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_writable<Iter, const T2&> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to, std::projected<Iter, Proj>, const T1*>)
 * constexpr void replace( Iter first, Sent last,
 *                         const T1& old_value, const T2& new_value, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range Range,
 *           class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_writable<std::ranges::iterator_t<Range>, const T2&> &&
 *            std::indirect_binary_predicate<std::ranges::equal_to,
 *                                           std::projected<std::ranges::iterator_t<Range>, Proj>, const T1*>)
 * constexpr void replace( Range&& r,
 *                         const T1& old_value, const T2& new_value, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to modify
 * - `r`             - the range of elements to modify
 * - `old_value`     - the value of elements to replace
 * - `new_value`     - the replacement value
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * (none)
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * comparisons and applications of `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::replace_if`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `replace_strategy` (`strategy_mode::autovec` on GCC, Clang, and ICPX).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range (1 range).
 * - **Auto-vectorization:** Inherits the `autovec` kernel from `raze::algorithm::replace_if` (see @ref autovec_behavior).
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::replace_if` using `raze::algorithm::equal_to`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/replace/Replace.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
 *     println(v);
 *
 *     // Replace all occurrences of 8 with 88
 *     raze::algorithm::replace(v, 8, 88);
 *     println(v);
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
 * - `raze::algorithm::replace_if`
 * - `raze::algorithm::replace_copy`
 * - `raze::algorithm::replace_copy_if`
*/
constexpr inline auto replace = options::function_with_traits<replace_t>[options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END