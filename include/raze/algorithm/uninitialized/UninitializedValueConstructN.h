#pragma once

#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_value_construct_n_t : Traits {
	template <detail::nothrow_forward_iterator It>
	constexpr raze_always_inline It operator()(It first, std::iter_difference_t<It> n) const
		requires(std::default_initializable<std::iter_value_t<It>>)
	{
		if (n <= 0) return first;
		return uninitialized_value_construct[Traits::traits()](
			std::counted_iterator<It>(std::move(first), n), std::default_sentinel).base();
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Value-constructs `n` objects in uninitialized storage with strong exception safety.
 *
 * 1) Value-initializes `n` elements starting at `first` via `std::construct_at`. If an exception is
 *    thrown, all previously constructed objects are destroyed. If `n <= 0`, does nothing.
 *
 * ### Declarations
 * ```cpp
 * template< nothrow_forward_iterator It >
 *   requires(std::default_initializable<std::iter_value_t<It>>)
 * constexpr It uninitialized_value_construct_n( It first, std::iter_difference_t<It> n );
 * ```
 *
 * ### Parameters
 * - `first` - beginning of uninitialized storage
 * - `n`     - number of objects to value-construct
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
 * - `raze::algorithm::uninitialized_value_construct`
 * - `raze::algorithm::uninitialized_default_construct_n`
 */
constexpr inline auto uninitialized_value_construct_n =
	options::function_with_traits<uninitialized_value_construct_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END

