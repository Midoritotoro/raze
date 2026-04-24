#pragma once 

#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Selectz  {
	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_	__mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (std::is_integral_v<_Mask_> && __has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_epi64(__mask, __as<__m128i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_epi32(__mask, __as<__m128i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_ps(__mask, __as<__m128>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_pd(__mask, __as<__m128d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_epi16(__mask, __as<__m128i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_maskz_mov_epi8(__mask, __as<__m128i>(__x)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (std::is_integral_v<_Mask_> && __has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_epi64(__mask, __as<__m256i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_epi32(__mask, __as<__m256i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_ps(__mask, __as<__m256>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_pd(__mask, __as<__m256d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_epi16(__mask, __as<__m256i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mov_epi8(__mask, __as<__m256i>(__x)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (std::is_integral_v<_Mask_> && __has_avx512f_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_epi64(__mask, __as<__m512i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_epi32(__mask, __as<__m512i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_ps(__mask, __as<__m512>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_pd(__mask, __as<__m512d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_epi16(__mask, __as<__m512i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mov_epi8(__mask, __as<__m512i>(__x)));
				}
			}
		}

		return _Select<_ISA_, _Type_>()(__x, _Zero<_ISA_, _Tp_>()(), __mask);
	}
};

__RAZE_VX_NAMESPACE_END
