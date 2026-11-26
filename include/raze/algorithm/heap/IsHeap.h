#pragma once

#include <raze/algorithm/heap/IsHeapUntil.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_heap_t : Traits {
	template <std::random_access_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
	{
		return is_heap_until[Traits::traits()](std::move(first), last,
			traits::fwd_fn(comp), traits::fwd_fn(proj)) == last;
	}

	template <std::ranges::random_access_range R,
		class Comp = std::less<>, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp,
			std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return (*this)(std::ranges::begin(r), std::ranges::end(r),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_heap
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether a range satisfies the max-heap property.
 *
 * 1) Returns `true` if `[first, last)` is a max-heap with respect to `comp` and `proj`, and `false` otherwise.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `is_heap(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
 * constexpr bool
 * is_heap( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr bool
 * is_heap( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range to examine
 * - `r`             - the random-access range to examine
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * `true` if the entire range forms a max-heap, `false` otherwise.
 *
 * ### Complexity
 * Linear: at most `max(N - 1, 0)` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/heap/IsHeap.h>
 *
 * int main() {
 *     const std::vector<int> v{9, 5, 4, 1, 1, 3};
 *     std::cout << std::boolalpha << raze::algorithm::is_heap(v) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::is_heap_until`
 * - `raze::algorithm::make_heap`
 */
constexpr inline auto is_heap = options::function_with_traits<is_heap_t>;

__RAZE_ALGORITHM_NAMESPACE_END

