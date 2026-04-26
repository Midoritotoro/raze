#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/mask/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Select {
	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (std::is_integral_v<_Mask_> && __has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_epi64(__as<__m128i>(__y), __mask, __as<__m128i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_epi32(__as<__m128i>(__y), __mask, __as<__m128i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_ps(__as<__m128>(__y), __mask, __as<__m128>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_pd(__as<__m128d>(__y), __mask, __as<__m128d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_epi16(__as<__m128i>(__y), __mask, __as<__m128i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_mask_mov_epi8(__as<__m128i>(__y), __mask, __as<__m128i>(__x)));
				}
			}
			else if constexpr (intrin_type<_Mask_>) {
				if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm_ternarylogic_epi32(__as<__m128i>(__mask), __as<__m128i>(__x), __as<__m128i>(__y), 0xCA));
				else if constexpr (__has_sse41_support_v<_ISA_>) {
					if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm_blendv_pd(__as<__m128d>(__y), __as<__m128d>(__x), __as<__m128d>(__mask)));
					else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm_blendv_ps(__as<__m128>(__y), __as<__m128>(__x), __as<__m128>(__mask)));
					else return __as<_Tp_>(_mm_blendv_epi8(__as<__m128i>(__y), __as<__m128i>(__x), __as<__m128i>(__mask)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (std::is_integral_v<_Mask_> && __has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_epi64(__as<__m256i>(__y), __mask, __as<__m256i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_epi32(__as<__m256i>(__y), __mask, __as<__m256i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_ps(__as<__m256>(__y), __mask, __as<__m256>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_pd(__as<__m256d>(__y), __mask, __as<__m256d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_epi16(__as<__m256i>(__y), __mask, __as<__m256i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_mask_mov_epi8(__as<__m256i>(__y), __mask, __as<__m256i>(__x)));
				}
			}
			else if constexpr (intrin_type<_Mask_>) {
				if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm256_ternarylogic_epi32(__as<__m256i>(__mask), __as<__m256i>(__x), __as<__m256i>(__y), 0xCA));
				else if constexpr (__has_avx2_support_v<_ISA_>) {
					if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm256_blendv_pd(__as<__m256d>(__y), __as<__m256d>(__x), __as<__m256d>(__mask)));
					else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_blendv_ps(__as<__m256>(__y), __as<__m256>(__x), __as<__m256>(__mask)));
					else return __as<_Tp_>(_mm256_blendv_epi8(__as<__m256i>(__y), __as<__m256i>(__x), __as<__m256i>(__mask)));
				}
				else if constexpr (__has_avx_support_v<_ISA_>) {
					if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm256_blendv_pd(__as<__m256d>(__y), __as<__m256d>(__x), __as<__m256d>(__mask)));
					else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_blendv_ps(__as<__m256>(__y), __as<__m256>(__x), __as<__m256>(__mask)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (std::is_integral_v<_Mask_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_epi64(__as<__m512i>(__y), __mask, __as<__m512i>(__x)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_epi32(__as<__m512i>(__y), __mask, __as<__m512i>(__x)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_ps(__as<__m512>(__y), __mask, __as<__m512>(__x)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_pd(__as<__m512d>(__y), __mask, __as<__m512d>(__x)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_epi16(__as<__m512i>(__y), __mask, __as<__m512i>(__x)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_mask_mov_epi8(__as<__m512i>(__y), __mask, __as<__m512i>(__x)));
				}
			}
			else if constexpr (intrin_type<_Mask_>) {
				return __as<_Tp_>(_mm512_ternarylogic_epi32(__as<__m512i>(__mask), __as<__m512i>(__x), __as<__m512i>(__y), 0xCA));
			}
		}

		if constexpr (arithmetic_type<_Tp_>)
			return __mask ? __x : __y;
		else
			return _Ternarylogic<_ISA_, _Type_>()(_To_vector<_ISA_, _Type_, _Tp_>(__mask), __x, __y, std::integral_constant<uint8, 0xca>{});
	}
};


__RAZE_VX_NAMESPACE_END
