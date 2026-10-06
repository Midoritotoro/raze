#pragma once 

#include <raze/algorithm/replace/ReplaceCopyIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_copy_t : Traits {
	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent,
		std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<InIt, OutIt> operator()(
		InIt first, Sent sent, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<InIt, OutIt> && std::indirectly_writable<OutIt, const T2&>)
	{
		return replace_copy_if[Traits::traits()](std::move(first), std::move(sent), std::move(out),
			algorithm::equal_to(function_return_type<Proj, std::iter_value_t<InIt>>(old)),
			v, traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt> operator()(
		R&& r, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> && std::indirectly_writable<OutIt, const T2&>)
	{
		return replace_copy_if[Traits::traits()](std::forward<R>(r), std::move(out),
			algorithm::equal_to(function_return_type<Proj, std::ranges::range_value_t<R>>(old)),
			v, traits::fwd_fn(proj));
	}
};

/**
 * @brief Copies elements from a range to another range, replacing all elements equal to a given value with a new value.
 *
 * 1) Copies elements from `[first, sent)` to the destination range beginning at `out`,
 *    replacing each element with `new_value` if `bool(std::invoke(proj, *i) == old_value)` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `replace_copy(std::ranges::begin(r), std::ranges::end(r), std::move(out), old_value, new_value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent,
 *           std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_copyable<InIt, OutIt> && std::indirectly_writable<OutIt, const T2&>)
 * constexpr std::ranges::unary_transform_result<InIt, OutIt>
 * replace_copy( InIt first, Sent sent, OutIt out,
 *               const T1& old_value, const T2& new_value, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable OutIt,
 *           class T1, class T2, class Proj = std::identity >
 *   requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> && std::indirectly_writable<OutIt, const T2&>)
 * constexpr std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt>
 * replace_copy( R&& r, OutIt out,
 *               const T1& old_value, const T2& new_value, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to copy from
 * - `r`           - the range of elements to copy from
 * - `out`         - the beginning of the destination range
 * - `old_value`   - the value of elements to replace
 * - `new_value`   - the replacement value
 * - `proj`        - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::unary_transform_result` containing:
 * - `in`: an iterator pointing to the end of the input range (`sent`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Exactly `sent - first` (or `std::ranges::distance(r)`) comparisons and applications of `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * replace_copy[raze::options::fscalar](...);
 * replace_copy[raze::options::unroll<2>](...);
 * replace_copy[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Implemented in terms of `replace_copy_if` using `raze::algorithm::equal_to`.
 * - If the source and destination model contiguous buffers, explicit SIMD vectorization (AVX-512, AVX2, SSE)
 *   or platform-specific autovectorization paths are utilized.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
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
 */
constexpr inline auto replace_copy = options::function_with_traits<replace_copy_t>[options::unroll<4>][replace_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
