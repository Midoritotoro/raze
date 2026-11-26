#pragma once

#include <raze/options/Options.h>
#include <memory>
#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct construct_at_t : Traits {
	template <class T, class... Args>
	constexpr raze_always_inline T* operator()(T* location, Args&&... args) const
		requires(requires { std::construct_at(location, std::forward<Args>(args)...); })
	{
		return std::construct_at(location, std::forward<Args>(args)...);
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Constructs an object of type `T` in-place at the storage pointed to by `location`.
 *
 * 1) Creates a `T` object at `location` initialized with `std::forward<Args>(args)...` via `std::construct_at`.
 *
 * ### Declarations
 * ```cpp
 * template< class T, class... Args >
 * constexpr T* construct_at( T* location, Args&&... args );
 * ```
 *
 * ### Parameters
 * - `location` - pointer to properly aligned uninitialized storage
 * - `args...`  - constructor arguments forwarded to `T`
 *
 * ### Return value
 * Pointer to the newly constructed object (`location`).
 *
 * ### Complexity
 * Constant (cost of constructing `T`).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::destroy_at`
 * - `raze::algorithm::uninitialized_value_construct`
 */
constexpr inline auto construct_at = options::function_with_traits<construct_at_t>;

__RAZE_ALGORITHM_NAMESPACE_END

