#pragma once 

#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>
#include <src/raze/vx/hw/x86/cast/SimdIntegralTypesCheck.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _MaskType_>
raze_nodiscard raze_always_inline auto __data(const _MaskType_& __mask) noexcept
	requires(__is_simd_mask_v<_MaskType_>)
{
	using _InternalMaskType = typename _MaskType_::mask_type;
	const auto __unwrapped_mask = static_cast<_InternalMaskType>(__mask);

	using _UnwrappedMaskType = decltype(__unwrapped_mask);

	if constexpr (__is_valid_simd_v<_UnwrappedMaskType>)
		return __data(__unwrapped_mask);
	else
		return __unwrapped_mask;
}

__RAZE_VX_NAMESPACE_END
