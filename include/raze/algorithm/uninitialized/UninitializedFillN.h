#pragma once

#include <raze/algorithm/uninitialized/UninitializedFill.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_fill_n_t : Traits {
	template <detail::nothrow_forward_iterator It, class T = std::iter_value_t<It>>
	constexpr raze_always_inline It operator()(It first, std::iter_difference_t<It> n, const T& x) const
		requires(std::constructible_from<std::iter_value_t<It>, const T&>)
	{
		if (n <= 0) return first;
		return uninitialized_fill[Traits::traits()](
			std::counted_iterator<It>(std::move(first), n), std::default_sentinel, x).base();
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Constructs `n` copies of a value in uninitialized storage.
 *
 * 1) Constructs `n` elements starting at `first` from `x` via `std::construct_at` with strong
 *    exception safety. If `n <= 0`, does nothing.
 *
 * ### Declarations
 * ```cpp
 * template< nothrow_forward_iterator It, class T = std::iter_value_t<It> >
 *   requires(std::constructible_from<std::iter_value_t<It>, const T&>)
 * constexpr It uninitialized_fill_n( It first, std::iter_difference_t<It> n, const T& x );
 * ```
 *
 * ### Parameters
 * - `first` - beginning of uninitialized storage
 * - `n`     - number of objects to construct
 * - `x`     - value to copy-construct (supports braced-init lists via C++26 P2248)
 *
 * ### Return value
 * Iterator past the last constructed object (`first + max(n, 0)`).
 *
 * ### Complexity
 * Linear in `n`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::uninitialized_fill`
 * - `raze::algorithm::fill_n`
 */
constexpr inline auto uninitialized_fill_n = options::function_with_traits<uninitialized_fill_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END

