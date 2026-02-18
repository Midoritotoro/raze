#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <typename _Type_>
__simd_nodiscard_inline_constexpr _Type_ abs(_Type_ __value) noexcept {
	static_assert(std::is_integral_v<_Type_> || std::is_floating_point_v<_Type_>);

	if constexpr (std::is_unsigned_v<_Type_>)
		return __value;
	else
		return (__value < 0) ? -__value : __value;
}

__RAZE_MATH_NAMESPACE_END
