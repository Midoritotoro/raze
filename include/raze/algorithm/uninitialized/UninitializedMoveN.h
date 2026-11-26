#pragma once

#include <raze/algorithm/uninitialized/UninitializedMove.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_move_n_t : Traits {
	template <std::input_iterator InIt, detail::nothrow_forward_iterator OutIt,
		detail::nothrow_sentinel_for<OutIt> Sent>
	constexpr raze_always_inline std::ranges::uninitialized_move_n_result<InIt, OutIt>
	operator()(InIt ifirst, std::iter_difference_t<InIt> n, OutIt ofirst, Sent olast) const
		requires(std::constructible_from<std::iter_value_t<OutIt>, std::iter_rvalue_reference_t<InIt>>)
	{
		if (n <= 0) return { std::move(ifirst), std::move(ofirst) };
		auto r = uninitialized_move[Traits::traits()](
			std::counted_iterator<InIt>(std::move(ifirst), n), std::default_sentinel,
			std::move(ofirst), std::move(olast));
		return { std::move(r.in).base(), std::move(r.out) };
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Move-constructs up to `n` objects into an uninitialized destination range.
 *
 * 1) Moves at most `n` elements starting at `ifirst` into uninitialized storage `[ofirst, olast)`
 *    with strong exception safety on the destination range.
 *
 * ### Declarations
 * ```cpp
 * template< std::input_iterator InIt, nothrow_forward_iterator OutIt, nothrow_sentinel_for<OutIt> Sent >
 *   requires(std::constructible_from<std::iter_value_t<OutIt>, std::iter_rvalue_reference_t<InIt>>)
 * constexpr std::ranges::uninitialized_move_n_result<InIt, OutIt>
 * uninitialized_move_n( InIt ifirst, std::iter_difference_t<InIt> n, OutIt ofirst, Sent olast );
 * ```
 *
 * ### Parameters
 * - `ifirst`          - beginning of source sequence
 * - `n`               - maximum number of elements to move
 * - `ofirst`, `olast` - uninitialized destination range
 *
 * ### Return value
 * A `std::ranges::uninitialized_move_n_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: `min(max(n, 0), distance(ofirst, olast))` move constructions.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::uninitialized_move`
 * - `raze::algorithm::uninitialized_copy_n`
 */
constexpr inline auto uninitialized_move_n = options::function_with_traits<uninitialized_move_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END

