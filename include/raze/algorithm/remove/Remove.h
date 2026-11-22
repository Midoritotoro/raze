#pragma once

#include <raze/algorithm/remove/RemoveIf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_t : Traits {
	template <std::permutable It, std::sentinel_for<It> Sent, class T = std::iter_value_t<It>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It> operator()(
		It first, Sent last, const std::type_identity_t<T>& value, Proj proj = {}) const
	{
		return remove_if[Traits::traits()](std::move(first), std::move(last),
			algorithm::equal_to(value), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class T = std::ranges::range_value_t<R>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R> operator()(
		R&& r, const std::type_identity_t<T>& value, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<R>>)
	{
		return remove_if[Traits::traits()](std::forward<R>(r),
			algorithm::equal_to(value), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Removes all elements equal to a specific value from a range in-place.
 *
 * 1) Removes all elements in `[first, last)` for which `bool(std::invoke(proj, *i) == value)` is `true`
 *    by shifting the remaining elements forward while preserving their relative order.
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `remove(std::ranges::begin(r), std::ranges::end(r), value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>, class Proj = std::identity >
 * constexpr std::ranges::subrange<It>
 * remove( It first, Sent last, const std::type_identity_t<T>& value, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T = std::ranges::range_value_t<R>, class Proj = std::identity >
 *   requires(std::permutable<std::ranges::iterator_t<R>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * remove( R&& r, const std::type_identity_t<T>& value, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to process
 * - `r`             - the range of elements to process
 * - `value`         - the value of elements to remove
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `[new_end, end)` spanning from the new logical end of the range
 * to the original end (`last` / `std::ranges::end(r)`), ready to be passed to a container's `erase()`.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * comparisons and applications of `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * remove[raze::options::fscalar](...);
 * remove[raze::options::fstatic](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Implemented in terms of `raze::algorithm::remove_if` using `raze::algorithm::equal_to`.
 * - Removing is done by shifting elements; the physical size of the underlying container is unchanged.
 * - For contiguous ranges, utilizes explicit SIMD `raze::vx::compress_store` (AVX-512, AVX2, SSSE3).
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/remove/Remove.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 2, 4, 2, 5};
 *
 *     auto tail = raze::algorithm::remove(v, 2);
 *     v.erase(tail.begin(), tail.end());
 *
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 1 3 4 5
 * ```
*/
constexpr inline auto remove = options::function_with_traits<remove_t>[remove_strategy];

__RAZE_ALGORITHM_NAMESPACE_END