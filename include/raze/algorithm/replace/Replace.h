#pragma once 

#include <raze/algorithm/replace/ReplaceIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_t : Traits {
	template <std::permutable Iter, std::sentinel_for<Iter> Sent,
		class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline void operator()(Iter first, Sent last,
		const T1& old_value, const T2& new_value, Proj proj = {}) const
	{
		replace_if[Traits::traits()](std::move(first), std::move(last), algorithm::equal_to(
			function_return_type<Proj, std::iter_value_t<Iter>>(old_value)),
			new_value, traits::fwd_fn(proj));
	}

	template <std::ranges::input_range Range, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline void operator()(Range&& r, const T1& old_value,
		const T2& new_value, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<Range>>)
	{
		replace_if[Traits::traits()](std::forward<Range>(r), algorithm::equal_to(
			function_return_type<Proj, std::ranges::range_value_t<Range>>(old_value)),
			new_value, traits::fwd_fn(proj));
	}
};

/**
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
 * template< std::permutable Iter, std::sentinel_for<Iter> Sent,
 *           class T1, class T2, class Proj = std::identity >
 * constexpr void replace( Iter first, Sent last,
 *                         const T1& old_value, const T2& new_value, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range Range,
 *           class T1, class T2, class Proj = std::identity >
 *   requires std::permutable<std::ranges::iterator_t<Range>>
 * constexpr void replace( Range&& r,
 *                         const T1& old_value, const T2& new_value, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to modify
 * - `r`           - the range of elements to modify
 * - `old_value`   - the value of elements to replace
 * - `new_value`   - the replacement value
 * - `proj`        - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * (none)
 *
 * ### Complexity
 * Exactly `last - first` (or `std::ranges::distance(r)`) comparisons and applications of `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * replace[raze::options::fscalar](...);
 * replace[raze::options::unroll<2>](...);
 * replace[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Implemented in terms of `replace_if` using `raze::algorithm::equal_to`.
 * - If the source models `contiguous_source`, explicit SIMD vectorization (AVX-512, AVX2, SSE)
 *   or platform-specific autovectorization paths are utilized.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
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
 */
constexpr inline auto replace = options::function_with_traits<replace_t>[options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END