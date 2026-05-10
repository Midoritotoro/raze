#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, std::unsigned_integral _Mask_>
raze_nodiscard raze_always_inline auto __to_k(_Mask_ __mask) noexcept {
	if constexpr (sizeof(_Mask_) == 1 && __has_avx512dq_support_v<_ISA_>) return _cvtmask8_u32(__mask);
	else if constexpr (sizeof(_Mask_) == 2 && __has_avx512f_support_v<_ISA_>) return _cvtmask16_u32(__mask);
	else if constexpr (sizeof(_Mask_) == 4 && __has_avx512bw_support_v<_ISA_>) return _cvtmask32_u32(__mask);
	else if constexpr (sizeof(_Mask_) == 8 && __has_avx512bw_support_v<_ISA_>) return _cvtmask64_u64(__mask);
	else return __mask;
}

__RAZE_VX_NAMESPACE_END
