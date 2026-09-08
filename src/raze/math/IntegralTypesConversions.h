#pragma once 

#include <numeric>
#include <src/raze/traits/TypeTraits.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::integral T>
raze_always_inline constexpr T min_limit() noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return 0;

	constexpr auto umax = static_cast<std::make_unsigned_t<T>>(-1);
	return static_cast<T>((umax >> 1) + 1);
}

template <class T>
raze_always_inline constexpr T max_limit() noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return static_cast<T>(-1);
	
	constexpr auto umax = static_cast<std::make_unsigned_t<T>>(-1);
	return static_cast<T>(umax >> 1);
}

__RAZE_MATH_NAMESPACE_END
