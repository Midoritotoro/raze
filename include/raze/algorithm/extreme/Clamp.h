#pragma once

#include <raze/options/Options.h>
#include <src/raze/utility/Assert.h>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct clamp_t : Traits {
	template <class T, class Proj = std::identity, class Comp = std::less<>>
	raze_nodiscard constexpr raze_always_inline const T&
	operator()(const T& v, const T& lo, const T& hi, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<const T*, Proj>>)
	{
		if !consteval {
			raze_debug_assert(!std::invoke(comp, std::invoke(proj, hi), std::invoke(proj, lo)));
		}

		if (std::invoke(comp, std::invoke(proj, v), std::invoke(proj, lo)))
			return lo;
		if (std::invoke(comp, std::invoke(proj, hi), std::invoke(proj, v)))
			return hi;
		return v;
	}
};

/**
 * @ingroup raze_algo_extreme
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Clamps a value to the inclusive range `[lo, hi]`.
 *
 * 1) Returns `lo` if `std::invoke(comp, std::invoke(proj, v), std::invoke(proj, lo))` is `true`,
 *    returns `hi` if `std::invoke(comp, std::invoke(proj, hi), std::invoke(proj, v))` is `true`,
 *    and otherwise returns `v`.
 *
 * ### Declarations
 * ```cpp
 * template< class T, class Proj = std::identity, class Comp = std::less<> >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<const T*, Proj>>)
 * constexpr const T&
 * clamp( const T& v, const T& lo, const T& hi, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `v`    - the value to clamp
 * - `lo`   - lower bound (must satisfy `!comp(proj(hi), proj(lo))`)
 * - `hi`   - upper bound
 * - `comp` - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj` - projection applied to `v`, `lo`, and `hi` (defaults to `std::identity`)
 *
 * ### Return value
 * Reference to `lo`, `hi`, or `v`.
 *
 * ### Complexity
 * Constant: at most 2 comparisons and 4 projections.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <raze/algorithm/extreme/Clamp.h>
 *
 * int main() {
 *     std::cout << raze::algorithm::clamp(15, 0, 10) << '\n';
 *     std::cout << raze::algorithm::clamp(-3, 0, 10) << '\n';
 *     std::cout << raze::algorithm::clamp(5, 0, 10) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - `raze::algorithm::min`
 * - `raze::algorithm::max`
 * - `raze::algorithm::minmax`
 */
constexpr inline auto clamp = options::function_with_traits<clamp_t>;

__RAZE_ALGORITHM_NAMESPACE_END

