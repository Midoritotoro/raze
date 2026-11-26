#pragma once

#include <raze/algorithm/uninitialized/Destroy.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct destroy_n_t : Traits {
	template <detail::nothrow_input_iterator It>
	constexpr raze_always_inline It operator()(It first, std::iter_difference_t<It> n) const noexcept
		requires(std::destructible<std::iter_value_t<It>>)
	{
		if (n <= 0) return first;
		return destroy[Traits::traits()](
			std::counted_iterator<It>(std::move(first), n), std::default_sentinel).base();
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Destroys the first `n` objects in a sequence.
 *
 * 1) Destroys the `n` objects starting at `first` via `std::destroy_at`. If `n <= 0`, does nothing.
 *
 * ### Declarations
 * ```cpp
 * template< nothrow_input_iterator It >
 *   requires(std::destructible<std::iter_value_t<It>>)
 * constexpr It destroy_n( It first, std::iter_difference_t<It> n ) noexcept;
 * ```
 *
 * ### Parameters
 * - `first` - beginning of sequence of objects to destroy
 * - `n`     - number of objects to destroy
 *
 * ### Return value
 * Iterator past the last destroyed object (`first + max(n, 0)`).
 *
 * ### Complexity
 * Linear in `n`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::destroy`
 * - `raze::algorithm::destroy_at`
 */
constexpr inline auto destroy_n = options::function_with_traits<destroy_n_t>;

__RAZE_ALGORITHM_NAMESPACE_END

