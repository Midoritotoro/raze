#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline bool __none_of(_Tp_ __x) noexcept {
	if constexpr (intrin_type<_Tp_>) {
		if constexpr (sizeof(_Tp_) == 16 && __has_sse41_support_v<_ISA_>) return _mm_testz_si128(__as<__m128i>(__x), __as<__m128i>(__x));
		else if constexpr (sizeof(_Tp_) == 32) return _mm256_testz_si256(__as<__m256i>(__x), __as<__m256i>(__x));
		else return _To_bitmask<_ISA_, _Type_>()(__x) == 0;
	}
	else if constexpr (std::is_integral_v<_Tp_> && !std::is_same_v<_Tp_, bool>) {
		if constexpr (sizeof(_Tp_) == 1 && __has_avx512dq_support_v<_ISA_>) return _kortestz_mask8_u8(__x, __x);
		else if constexpr (sizeof(_Tp_) == 2 && __has_avx512f_support_v<_ISA_>) return _kortestz_mask16_u8(__x, __x);
		else if constexpr (sizeof(_Tp_) == 4 && __has_avx512bw_support_v<_ISA_>) return _kortestz_mask32_u8(__x, __x);
		else if constexpr (sizeof(_Tp_) == 8 && __has_avx512bw_support_v<_ISA_>) return _kortestz_mask64_u8(__x, __x);
		else return (__x == 0);
	}
	else {
		return !__x;
	}
}

template <arch::ISA _ISA_, arithmetic_type _Type_, raw_mask_type _Tp_, raw_mask_type _Mask_>
raze_nodiscard raze_always_inline bool __none_of(_Tp_ __x, _Mask_ __mask) noexcept 
	requires((intrin_type<_Tp_> && intrin_type<_Mask_>) || (std::unsigned_integral<_Tp_> && std::unsigned_integral<_Mask_>))
{
	if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) { return !__mask || !__x; }
	else return __none_of<_ISA_, _Type_>(_Mask_and<_ISA_, _Type_>()(__x, __mask));
}

__RAZE_VX_NAMESPACE_END
