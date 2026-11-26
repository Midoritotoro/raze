#pragma once

#include <raze/options/Options.h>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class It>
concept nothrow_input_iterator =
	std::input_iterator<It> &&
	std::is_lvalue_reference_v<std::iter_reference_t<It>> &&
	std::same_as<std::remove_cvref_t<std::iter_reference_t<It>>, std::iter_value_t<It>>;

template <class Sent, class It>
concept nothrow_sentinel_for = std::sentinel_for<Sent, It>;

template <class It>
concept nothrow_forward_iterator =
	nothrow_input_iterator<It> &&
	std::forward_iterator<It> &&
	nothrow_sentinel_for<It, It>;

template <class R>
concept nothrow_input_range =
	std::ranges::input_range<R> &&
	nothrow_input_iterator<std::ranges::iterator_t<R>> &&
	nothrow_sentinel_for<std::ranges::sentinel_t<R>, std::ranges::iterator_t<R>>;

template <class R>
concept nothrow_forward_range =
	nothrow_input_range<R> &&
	nothrow_forward_iterator<std::ranges::iterator_t<R>>;

template <class It>
struct uninit_destroy_guard {
	It first;
	It& current;
	bool active = true;

	constexpr ~uninit_destroy_guard() {
		if (active) {
			for (; first != current; ++first)
				std::destroy_at(std::addressof(*first));
		}
	}
};

} // namespace detail

template <class Traits>
struct destroy_at_t : Traits {
	template <std::destructible T>
	constexpr raze_always_inline void operator()(T* location) const noexcept {
		std::destroy_at(location);
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Destroys an object (or array of objects) at a given address.
 *
 * 1) If `T` is an array type, destroys its elements in reverse order; otherwise invokes `location->~T()`.
 *
 * ### Declarations
 * ```cpp
 * template< std::destructible T >
 * constexpr void destroy_at( T* location ) noexcept;
 * ```
 *
 * ### Parameters
 * - `location` - pointer to the object to destroy
 *
 * ### Return value
 * (none)
 *
 * ### Complexity
 * Constant for non-array types; linear in array extent for array types.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::destroy`
 * - `raze::algorithm::destroy_n`
 * - `raze::algorithm::construct_at`
 */
constexpr inline auto destroy_at = options::function_with_traits<destroy_at_t>;

__RAZE_ALGORITHM_NAMESPACE_END

