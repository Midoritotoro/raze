#pragma once

#include <raze/Types.h>
#include <src/raze/RazeNamespace.h>
#include <src/raze/compatibility/Inline.h>

#include <concepts>
#include <type_traits>
#include <utility>

__RAZE_MATH_NAMESPACE_BEGIN

template <class T>
concept __strict_integer = std::integral<T> &&
	!std::same_as<T, bool> && !std::same_as<T, char> && !std::same_as<T, wchar_t> &&
	!std::same_as<T, char8_t> && !std::same_as<T, char16_t> && !std::same_as<T, char32_t>;

template <class T1, class T2>
	requires (std::is_arithmetic_v<T1> && std::is_arithmetic_v<T2>)
raze_always_inline constexpr auto max(const T1& x, const T2& y) noexcept {
	if constexpr (__strict_integer<T1> && __strict_integer<T2> && std::is_signed_v<T1> != std::is_signed_v<T2>) {
		using U = std::common_type_t<T1, T2>;
		return std::cmp_less(x, y) ? static_cast<U>(y) : static_cast<U>(x);
	}
	else {
		return (x < y) ? y : x;
	}
}

template <class T1, class T2>
raze_always_inline constexpr auto min(const T1& x, const T2& y) noexcept {
	if constexpr (__strict_integer<T1> && __strict_integer<T2> && std::is_signed_v<T1> != std::is_signed_v<T2>) {
		using S = std::conditional_t<std::is_signed_v<T1>, T1, T2>;
		using U = std::conditional_t<std::is_unsigned_v<T1>, T1, T2>;
		using R = std::conditional_t<(sizeof(S) > sizeof(U)), S,
			std::conditional_t<(sizeof(U) < sizeof(i64)), i64, S>>;
		return std::cmp_greater(x, y) ? static_cast<R>(y) : static_cast<R>(x);
	}
	else {
		return (x > y) ? y : x;
	}
}

__RAZE_MATH_NAMESPACE_END

