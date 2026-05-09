#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Mul {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mul_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mul_ps(__as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__has_avx512dq_support_v<_ISA_> && __has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm_mullo_epi64(__as<__m128i>(__x), __as<__m128i>(__y)));
				else {
					const auto __left_high = _mm_srli_epi64(__as<__m128i>(__x), 32);
					const auto __right_high = _mm_srli_epi64(__as<__m128i>(__y), 32);

					const auto __multiplied1 = _mm_mul_epu32(__left_high, __as<__m128i>(__y));
					const auto __multiplied2 = _mm_mul_epu32(__right_high, __as<__m128i>(__x));

					const auto __add64 = _mm_add_epi64(__multiplied2, __multiplied1);

					const auto __high = _mm_slli_epi64(__add64, 32);
					const auto __low = _mm_mul_epu32(__as<__m128i>(__x), __as<__m128i>(__y));

					return __as<_Tp_>(_mm_add_epi64(__low, __high));
				}
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__has_sse41_support_v<_ISA_>) return __as<_Tp_>(_mm_mullo_epi32(__as<__m128i>(__x), __as<__m128i>(__y)));
				else {
					const auto __shuffled_left = _mm_shuffle_epi32(__as<__m128i>(__x), 0xF5);
					const auto __shuffled_right = _mm_shuffle_epi32(__as<__m128i>(__y), 0xF5);

					const auto __product_02 = _mm_mul_epu32(__as<__m128i>(__x), __as<__m128i>(__y));
					const auto __product_13 = _mm_mul_epu32(__shuffled_left, __shuffled_right);

					const auto __product_01 = _mm_unpacklo_epi32(__product_02, __product_13);
					const auto __product_23 = _mm_unpackhi_epi32(__product_02, __product_13);

					return __as<_Tp_>(_mm_unpacklo_epi64(__product_01, __product_23));
				}
			}
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mullo_epi16(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
				if constexpr (__has_avx512bw_support_v<_ISA_> && __has_avx512vl_support_v<_ISA_>) {
					const auto __converted_x = _mm256_cvtepu8_epi16(__as<__m128i>(__x));
					const auto __converted_y = _mm256_cvtepu8_epi16(__as<__m128i>(__y));

					const auto __multiplied = _mm256_mullo_epi16(__converted_x, __converted_y);
					return __as<_Tp_>(_mm256_cvtepi16_epi8(__multiplied));
				}
				else if constexpr (__has_ssse3_support_v<_ISA_>) {
					const auto __low_byte_mask = _mm_set1_epi16(0x00FF);

					auto __product_low_words = _mm_mullo_epi16(__as<__m128i>(__y), __as<__m128i>(__x));
					__product_low_words = _mm_and_si128(__product_low_words, __low_byte_mask);

					const auto __inverted_mask_and_x = _mm_andnot_si128(__low_byte_mask, __as<__m128i>(__x));
					auto __widened_byte_products = _mm_maddubs_epi16(__as<__m128i>(__y), __inverted_mask_and_x);

					return __as<_Tp_>(_mm_or_si128(__product_low_words, _mm_slli_epi16(__widened_byte_products, 8)));
				}
				else {
					const auto __left_odd = _mm_srli_epi16(__as<__m128i>(__x), 8);
					const auto __right_odd = _mm_srli_epi16(__as<__m128i>(__y), 8);

					const auto __multiplied_even = _mm_mullo_epi16(__as<__m128i>(__x), __as<__m128i>(__y));
					const auto __multiplied_odd = _mm_slli_epi16(_mm_mullo_epi16(__left_odd, __right_odd), 8);

					return __as<_Tp_>(_Select<_ISA_,  _Type_>()(__multiplied_even, __multiplied_odd, _mm_set1_epi32(0x00FF00FF)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mul_ps(__as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mul_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__has_avx512bw_support_v<_ISA_> && __has_avx512vl_support_v<_ISA_> && (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)) {
					const auto __converted_x = _mm512_cvtepu8_epi16(__as<__m256i>(__x));
					const auto __converted_y = _mm512_cvtepu8_epi16(__as<__m256i>(__y));

					const auto __multiplied = _mm512_mullo_epi16(__converted_x, __converted_y);
					return __as<_Tp_>(_mm512_cvtepi16_epi8(__multiplied));
				}
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
					const auto __low_byte_mask = _mm256_set1_epi16(0x00FF);
					const auto __inverted_mask_and_vector_x = _mm256_andnot_si256(__low_byte_mask, __as<__m256i>(__x));

					auto __product_of_words = _mm256_mullo_epi16(__as<__m256i>(__y), __as<__m256i>(__x));
					__product_of_words = _mm256_and_si256(__product_of_words, __low_byte_mask);

					auto __widened_byte_products = _mm256_maddubs_epi16(__as<__m256i>(__y), __inverted_mask_and_vector_x);
					__widened_byte_products = _mm256_slli_epi16(__widened_byte_products, 8);

					return __as<_Tp_>(_mm256_or_si256(__product_of_words, __widened_byte_products));
				}
				else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mullo_epi16(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mullo_epi32(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
					const auto __x_shifted_high = _mm256_srli_epi64(__as<__m256i>(__x), 32);
					const auto __y_shifted_high = _mm256_srli_epi64(__as<__m256i>(__y), 32);

					const auto __product_low_low = _mm256_mul_epu32(__as<__m256i>(__x), __as<__m256i>(__y));
					const auto __product_high_low = _mm256_mul_epu32(__y_shifted_high, __as<__m256i>(__x));
					const auto __product_low_high = _mm256_mul_epu32(__x_shifted_high, __as<__m256i>(__y));

					const auto __sum_cross_terms = _mm256_add_epi64(__product_high_low, __product_low_high);
					const auto __shifted_cross_terms = _mm256_slli_epi64(__sum_cross_terms, 32);

					return __as<_Tp_>(_mm256_add_epi64(__product_low_low, __shifted_cross_terms));
				}
			}
			else {
				const auto __low = _Mul<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __as<__m128i>(__y));
				const auto __high = _Mul<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1),
					_mm256_extractf128_si256(__as<__m256i>(__y), 1));

				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__has_avx512dq_support_v<_ISA_>) return __as<_Tp_>(_mm512_mullo_epi64(__as<__m512i>(__x), __as<__m512i>(__y)));
				else {
					const auto __left_high = _mm512_srli_epi64(__as<__m512i>(__x), 32);
					const auto __right_high = _mm512_srli_epi64(__as<__m512i>(__y), 32);

					const auto __multiplied1 = _mm512_mul_epu32(__left_high, __as<__m512i>(__y));
					const auto __multiplied2 = _mm512_mul_epu32(__right_high, __as<__m512i>(__x));

					const auto __add64 = _mm512_add_epi64(__multiplied2, __multiplied1);

					const auto __high = _mm512_slli_epi64(__add64, 32);
					const auto __low = _mm512_mul_epu32(__as<__m512i>(__x), __as<__m512i>(__y));

					return __as<_Tp_>(_mm512_add_epi64(__low, __high));
				}
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mullo_epi32(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mul_ps(__as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mul_pd(__as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
					return __as<_Tp_>(_mm512_mullo_epi16(__as<__m512i>(__x), __as<__m512i>(__y)));
				}
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
					const auto __mul1 = _mm512_mullo_epi16(__as<__m512i>(__x), __as<__m512i>(__y));
					const auto __and_mask = _mm512_set1_epi32(0x00FF00FF);

					const auto __8bits = _mm512_andnot_si512(__and_mask, __as<__m512i>(__x));
					const auto __multiplied = _mm512_maddubs_epi16(__as<__m512i>(__y), __8bits);

					const auto __shifted = _mm512_slli_epi16(__multiplied, 8);
					return __as<_Tp_>(_mm512_ternarylogic_epi64(__shifted, __mul1, __and_mask, 0xF8));
				}
			}
			else {
				const auto __low = _Mul<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x), __as<__m256i>(__y));
				const auto __high = _Mul<arch::ISA::AVX2, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__x), 1),
					_mm512_extractf64x4_pd(__as<__m512d>(__y), 1));

				return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__low), __as<__m256i>(__high), 1));
			}
		}
		else {
			return __x * __y;
		}
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_mul_ps(__mask, __as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_mul_pd(__mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_mullo_epi32(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm_maskz_mullo_epi64(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm_maskz_mullo_epi16(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)) {
				const auto __converted_x = _mm256_cvtepu8_epi16(__as<__m128i>(__x));
				const auto __converted_y = _mm256_cvtepu8_epi16(__as<__m128i>(__y));

				const auto __multiplied = _mm256_mullo_epi16(__converted_x, __converted_y);
				return __as<_Tp_>(_mm256_maskz_cvtepi16_epi8(__mask, __multiplied));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mul_ps(__mask, __as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mul_pd(__mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_mullo_epi32(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm256_maskz_mullo_epi64(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm256_maskz_mullo_epi16(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)) {
				const auto __converted_x = _mm512_cvtepu8_epi16(__as<__m256i>(__x));
				const auto __converted_y = _mm512_cvtepu8_epi16(__as<__m256i>(__y));

				const auto __multiplied = _mm512_mullo_epi16(__converted_x, __converted_y);
				return __as<_Tp_>(_mm512_maskz_cvtepi16_epi8(__mask, __multiplied));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64 && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mul_ps(__mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mul_pd(__mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_mullo_epi32(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm512_maskz_mullo_epi64(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm512_maskz_mullo_epi16(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_mul_ps(__as<__m128>(__src), __mask, __as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_mul_pd(__as<__m128d>(__src), __mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_mullo_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm_mask_mullo_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm_mask_mullo_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)) {
				const auto __converted_x = _mm256_cvtepu8_epi16(__as<__m128i>(__x));
				const auto __converted_y = _mm256_cvtepu8_epi16(__as<__m128i>(__y));

				const auto __multiplied = _mm256_mullo_epi16(__converted_x, __converted_y);
				return __as<_Tp_>(_mm256_mask_cvtepi16_epi8(__as<__m128i>(__src), __mask, __multiplied));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_mul_ps(__as<__m256>(__src), __mask, __as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_mul_pd(__as<__m256d>(__src), __mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_mullo_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm256_mask_mullo_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm256_mask_mullo_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)) {
				const auto __converted_x = _mm512_cvtepu8_epi16(__as<__m256i>(__x));
				const auto __converted_y = _mm512_cvtepu8_epi16(__as<__m256i>(__y));

				const auto __multiplied = _mm512_mullo_epi16(__converted_x, __converted_y);
				return __as<_Tp_>(_mm512_mask_cvtepi16_epi8(__as<__m256i>(__src), __mask, __multiplied));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64 && std::is_integral_v<_Mask_>) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_mul_ps(__as<__m512>(__src), __mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_mul_pd(__as<__m512d>(__src), __mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_mullo_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__has_avx512dq_support_v<_ISA_> && (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)) {
				return __as<_Tp_>(_mm512_mask_mullo_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
			else if constexpr (__has_avx512bw_support_v<_ISA_> && (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)) {
				return __as<_Tp_>(_mm512_mask_mullo_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
