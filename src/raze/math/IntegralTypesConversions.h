#pragma once 

#include <numeric>
#include <src/raze/type_traits/TypeTraits.h>

__RAZE_MATH_NAMESPACE_BEGIN


template <class _Type_>
constexpr _Type_ __minimum_integral_limit() noexcept {
	static_assert(std::is_integral_v<_Type_>);

	if constexpr (std::is_unsigned_v< _Type_>)
		return 0;

	constexpr auto __unsigned_max = static_cast<std::make_unsigned_t<_Type_>>(-1);
	return static_cast<_Type_>((__unsigned_max >> 1) + 1);
}

template <class _Type_>
constexpr _Type_ __maximum_integral_limit() noexcept { 
	static_assert(std::is_integral_v<_Type_>);

	if constexpr (std::is_unsigned_v<_Type_>)
		return static_cast<_Type_>(-1);
	
	constexpr auto __unsigned_max = static_cast<std::make_unsigned_t<_Type_>>(-1);
	return static_cast<_Type_>(__unsigned_max >> 1);
}

template <
	typename _TypeTo_,
	typename _TypeFrom_,
	std::enable_if_t<std::is_integral_v<_TypeFrom_> && std::is_integral_v<_TypeTo_>
		&& !std::is_same_v<_TypeFrom_, _TypeTo_>, int> = 0>
constexpr inline bool __convert_integral(
	const _TypeFrom_	__from,
	_TypeTo_&			__to) noexcept
{
	if constexpr (std::is_same_v<_TypeFrom_, _TypeTo_>) {
		__to = __from;
		return true;
	}

	constexpr auto __to_maximum_limit = __maximum_integral_limit<_TypeTo_>();
	constexpr auto __to_minimum_limit = __minimum_integral_limit<_TypeTo_>();

	if constexpr (std::is_signed_v<_TypeFrom_> && std::is_signed_v<_TypeTo_>) {
		if constexpr (sizeof(_TypeFrom_) > sizeof(_TypeTo_))
			if (__from > __to_maximum_limit || __from < __to_minimum_limit)
				return false;
	}
	else if (std::is_unsigned_v<_TypeFrom_> && std::is_unsigned_v<_TypeTo_>) {
		if constexpr (sizeof(_TypeFrom_) > sizeof(_TypeTo_))
			if (__from > __to_maximum_limit)
				return false;
	}
	else if (std::is_signed_v<_TypeFrom_> && std::is_unsigned_v<_TypeTo_>) {
		if constexpr (sizeof(_TypeFrom_) > sizeof(_TypeTo_))
			if (__from < __to_minimum_limit)
				return false;
	}
	else { /* std::is_unsigned_v<_TypeFrom_> && std::is_signed_v<_TypeTo_> */
		if constexpr (sizeof(_TypeFrom_) > sizeof(_TypeTo_))
			if (__from > __to_maximum_limit)
				return false;
	}

	__to = static_cast<_TypeTo_>(__from);
	return true;
}

__RAZE_MATH_NAMESPACE_END
