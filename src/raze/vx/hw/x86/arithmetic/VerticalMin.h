#pragma once 

#include <src/raze/vx/hw/x86/compare/Less.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Vertical_min {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_min_epi16(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm_min_epu8(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_min_ps(__as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_min_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__has_sse41_support_v<_ISA_>) {
				if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_min_epi32(__as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_min_epu32(__as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_min_epu16(__as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_min_epi8(__as<__m128i>(__x), __as<__m128i>(__y)));
			}
			else if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_min_epi64(__as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_min_epu64(__as<__m128i>(__x), __as<__m128i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_min_ps(__as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_min_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_min_epi32(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_min_epu32(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_min_epi16(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_min_epu16(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_min_epi8(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_min_epu8(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_min_ps(__as<__m256>(__x), __as<__m256>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_min_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			}
			else if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_min_epi64(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_min_epu64(__as<__m256i>(__x), __as<__m256i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_min_epi64(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_min_epu64(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_min_epi32(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_min_epu32(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_min_ps(__as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_min_pd(__as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_min_epi8(__as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_min_epu8(__as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_min_epi16(__as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_min_epu16(__as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()(__x, __y, _Less<_ISA_, _Type_>()(__x, __y));
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epi64(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epu64(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epi32(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epu32(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_ps(__mask, __as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_pd(__mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epi16(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epu16(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epi8(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm_maskz_min_epu8(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epi64(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epu64(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epi32(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epu32(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_ps(__mask, __as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_pd(__mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epi16(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epu16(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epi8(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_min_epu8(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epi64(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epu64(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epi32(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epu32(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_ps(__mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_pd(__mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epi16(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epu16(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epi8(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_min_epu8(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epu64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epu32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_min_ps(__as<__m128>(__src), __mask, __as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_min_pd(__as<__m128d>(__src), __mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epu16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epi8(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm_mask_min_epu8(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epu64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epu32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_ps(__as<__m256>(__src), __mask, __as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_pd(__as<__m256d>(__src), __mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epu16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epi8(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_mask_min_epu8(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epu64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epu32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_ps(__as<__m512>(__src), __mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_pd(__as<__m512d>(__src), __mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epu16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epi8(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_mask_min_epu8(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END