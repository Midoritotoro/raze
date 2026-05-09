#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Right_shift {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_>) {
				if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm_sra_epi64(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
				else {
					auto __high_shifted = _mm_setzero_si128();
					auto __low_shifted = _mm_setzero_si128();

					if (__shift <= 32) {
						const auto __shift_vector = _mm_cvtsi32_si128(__shift);
						__high_shifted = _mm_sra_epi32(__as<__m128i>(__x), __shift_vector);
						__low_shifted = _mm_srl_epi64(__as<__m128i>(__x), __shift_vector);
					}
					else {
						const auto __shift_vector = _mm_cvtsi32_si128(static_cast<i32>(__shift) - 32);
						__high_shifted = _mm_srai_epi32(__as<__m128i>(__x), 31);
						__low_shifted = _mm_srli_epi64(_mm_sra_epi32(__as<__m128i>(__x), __shift_vector), 32);
					}

					return __as<_Tp_>(_Select<_ISA_, _Type_>()(__high_shifted, __low_shifted, _mm_set_epi32(-1, 0, -1, 0)));
				}
			}
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_srl_epi64(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_sra_epi32(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_srl_epi32(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_sra_epi16(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_srl_epi16(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm_srl_epi16(
				_mm_and_si128(__as<__m128i>(__x), _mm_set1_epi8(0xFFu << __shift)), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi8_v<_Type_>) {
				const auto __even_shifted = _mm_sra_epi16(_mm_slli_epi16(__as<__m128i>(__x), 8), _mm_cvtsi32_si128(__shift + 8));
				const auto __odd_shifted = _mm_sra_epi16(__as<__m128i>(__x), _mm_cvtsi32_si128(__shift));

				if constexpr (__has_avx512vl_support_v<_ISA_> && __has_avx512bw_support_v<_ISA_>) return __as<_Tp_>(_mm_mask_mov_epi8(__odd_shifted, 0x5555, __even_shifted));
				else return __as<_Tp_>(_Select<_ISA_, _Type_>()(__even_shifted, __odd_shifted, _mm_set1_epi32(0x00FF00FF)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_>) {
					if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm256_sra_epi64(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
					else {
						auto __high_shifted = _mm256_setzero_si256();
						auto __low_shifted = _mm256_setzero_si256();

						if (__shift <= 32) {
							const auto __shift_vector = _mm_cvtsi32_si128(__shift);
							__high_shifted = _mm256_sra_epi32(__as<__m256i>(__x), __shift_vector);
							__low_shifted = _mm256_srl_epi64(__as<__m256i>(__x), __shift_vector);
						}
						else {
							__high_shifted = _mm256_srai_epi32(__as<__m256i>(__x), 31);
							__low_shifted = _mm256_srli_epi64(_mm256_sra_epi32(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift - 32)), 32);
						}

						return __as<_Tp_>(_mm256_blend_epi32(__low_shifted, __high_shifted, 0xAA));
					}
				}
				else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_srl_epi64(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_sra_epi32(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_srl_epi32(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_sra_epi16(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_srl_epi16(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi8_v<_Type_>) {
					const auto __even_shifted = _mm256_sra_epi16(_mm256_slli_epi16(__as<__m256i>(__x), 8), _mm_cvtsi32_si128(__shift + 8));
					const auto __odd_shifted = _mm256_sra_epi16(__as<__m256i>(__x), _mm_cvtsi32_si128(__shift));

					if constexpr (__has_avx512vl_support_v<_ISA_> && __has_avx512bw_support_v<_ISA_>) return __as<_Tp_>(_mm256_mask_mov_epi8(__odd_shifted, 0x55555555, __even_shifted));
					else return __as<_Tp_>(_Select<_ISA_, _Type_>()(__even_shifted, __odd_shifted, _mm256_set1_epi32(0x00FF00FF)));
				}
				else if constexpr (__is_epu8_v<_Type_>) {
					const auto __and_mask = _mm256_and_si256(__as<__m256i>(__x), _mm256_set1_epi8(0xFFu << __shift));
					return __as<_Tp_>(_mm256_srl_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
				}
			}
			else {
				const auto __low = _Right_shift<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __shift);
				const auto __high = _Right_shift<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1), __shift);

				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_sra_epi64(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_srl_epi64(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_sra_epi32(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_srl_epi32(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_sra_epi16(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_srl_epi16(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_srl_epi16(
					_mm512_and_si512(__as<__m512i>(__x), _mm512_set1_epi8(0xFFu << __shift)), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epi8_v<_Type_>) {
					const auto __even_vector = _mm512_slli_epi16(__as<__m512i>(__x), 8);

					const auto __even_shifted = _mm512_sra_epi16(__even_vector, _mm_cvtsi32_si128(__shift + 8));
					const auto __odd_shifted = _mm512_sra_epi16(__as<__m512i>(__x), _mm_cvtsi32_si128(__shift));

					return __as<_Tp_>(_mm512_mask_mov_epi8(__even_shifted, 0xAAAAAAAAAAAAAAAA, __odd_shifted));
				}
			}
			else {
				const auto __low = __as<__m256i>(__x);
				const auto __high = _mm512_extracti64x4_epi64(__as<__m512i>(__x), 1);

				const auto __low_shifted = _Right_shift<arch::ISA::AVX2, _Type_>()(__low, __shift);
				const auto __high_shifted = _Right_shift<arch::ISA::AVX2, _Type_>()(__high, __shift);

				return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__low_shifted), __high_shifted, 1));
			}
		}
		else {
			_Tp_ __result = 0;

			if (__shift < sizeof(_Type_) * 8) {
				__result = __x >> __shift;
			}
			else {
				if constexpr (std::is_signed_v<_Type_>)
					__result = __x >> (sizeof(_Type_) * 8 - 1);
			}

			return __result;
		}
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_maskz_sra_epi64(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_srl_epi64(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_maskz_sra_epi32(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_srl_epi32(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_maskz_sra_epi16(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_srl_epi16(__mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sra_epi64(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_srl_epi64(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sra_epi32(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_srl_epi32(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_sra_epi16(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_srl_epi16(__mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sra_epi64(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_srl_epi64(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sra_epi32(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_srl_epi32(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_sra_epi16(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_srl_epi16(__mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __shift), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, u32 __shift, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_mask_sra_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_srl_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_mask_sra_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_srl_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_mask_sra_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_srl_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_mask_sra_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_srl_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_mask_sra_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_srl_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_mask_sra_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_srl_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_mask_sra_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_srl_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_mask_sra_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_srl_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_mask_sra_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
				else if constexpr (__is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_srl_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), _mm_cvtsi32_si128(__shift)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __shift), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
