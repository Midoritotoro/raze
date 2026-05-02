#pragma once 

#include <src/raze/vx/hw/x86/merge/Selectz.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Left_shift {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_sll_epi64(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_sll_epi32(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_sll_epi16(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_sll_epi16(
				_mm_and_si128(__as<__m128i>(__x), _mm_set1_epi8(0xFFu >> __shift)), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_sll_epi64(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_sll_epi32(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_sll_epi16(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_sll_epi16(
					_mm256_and_si256(__as<__m256i>(__x), _mm256_set1_epi8(0xFFu >> __shift)), _mm_cvtsi32_si128(__shift)));
			}
			else {
				const auto __low = _Left_shift<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __shift);
				const auto __high = _Left_shift<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1), __shift);

				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_sll_epi64(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_sll_epi32(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_sll_epi16(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_sll_epi16(
					_mm512_and_si512(__as<__m512i>(__x), _mm512_set1_epi8(0xFFull >> __shift)), _mm_cvtsi32_si128(__shift)));
			}
			else {
				const auto __low = __as<__m256i>(__x);
				const auto __high = _mm512_extracti64x4_epi64(__as<__m512i>(__x), 1);

				const auto __low_shifted = _Left_shift<arch::ISA::AVX2, _Type_>()(__low, __shift);
				const auto __high_shifted = _Left_shift<arch::ISA::AVX2, _Type_>()(__high, __shift);

				return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__low_shifted), __high_shifted, 1));
			}
		}
		else {
			return (__shift < sizeof(_Tp_) * 8) ? (__x << __shift) : 0;
		}
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_sll_epi64(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_sll_epi32(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_sll_epi16(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sll_epi64(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sll_epi32(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sll_epi16(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sll_epi64(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sll_epi32(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sll_epi16(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}

		return _Selectz<_ISA_, _Type_>()((*this)(__x, __shift), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_sll_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_sll_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_sll_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_sll_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_sll_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_sll_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_sll_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_sll_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_sll_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __shift), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
