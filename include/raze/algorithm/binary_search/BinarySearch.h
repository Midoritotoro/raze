#pragma once

#include <raze/algorithm/binary_search/LowerBound.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct binary_search_t : Traits {
	template <std::forward_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<It, Proj>>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(It first, Sent last, const T& value, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
	{
		auto it = lower_bound[Traits::traits()](std::move(first), last, value,
			traits::fwd_fn(comp), traits::fwd_fn(proj));
		return it != last && !std::invoke(comp, value, std::invoke(proj, *it));
	}

	template <std::ranges::forward_range R,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<std::ranges::iterator_t<R>, Proj>>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, const T*,
			std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return (*this)(std::ranges::begin(r), std::ranges::end(r), value,
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_binary_search
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether an element equivalent to a given value exists in a sorted range.
 *
 * 1) Performs a binary search on `[first, last)` and returns `true` if there is an element equivalent
 *    to `value` with respect to `comp` and `proj`, and `false` otherwise.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `binary_search(std::ranges::begin(r), std::ranges::end(r), value, comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<It, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
 * constexpr bool
 * binary_search( It first, Sent last, const T& value, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<std::ranges::iterator_t<R>, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr bool
 * binary_search( R&& r, const T& value, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the sorted range to search
 * - `r`             - the sorted range to search
 * - `value`         - value to search for (supports braced-init lists via C++26 P2248)
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to range elements (defaults to `std::identity`)
 *
 * ### Return value
 * `true` if an equivalent element exists in the range, `false` otherwise.
 *
 * ### Complexity
 * Logarithmic: at most `log2(N) + 2` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/binary_search/BinarySearch.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 3, 5, 7, 9};
 *     std::cout << std::boolalpha << raze::algorithm::binary_search(v, 5) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::lower_bound`
 * - `raze::algorithm::upper_bound`
 * - `raze::algorithm::equal_range`
 */
constexpr inline auto binary_search = options::function_with_traits<binary_search_t>;

__RAZE_ALGORITHM_NAMESPACE_END

