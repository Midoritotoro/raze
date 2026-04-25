#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Abs {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x) const noexcept {
		if constexpr (std::is_unsigned_v<_Type_>)
			return __x;

		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_>) {
				if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm_abs_epi64(__as<__m128i>(__x)));
				else {
					const auto __high_sign = _mm_srai_epi32(__as<__m128i>(__x), 31);
					const auto __sign = _mm_shuffle_epi32(__high_sign, 0xF5);

					const auto __invert = _mm_xor_si128(__as<__m128i>(__x), __sign);
					return __as<_Tp_>(_mm_sub_epi64(__invert, __sign));
				}
			}
			else if constexpr (__is_epi32_v<_Type_>) {
				if constexpr (__has_ssse3_support_v<_ISA_>) return __as<_Tp_>(_mm_abs_epi32(__as<__m128i>(__x)));
				else {
					const auto __sign = _mm_srai_epi32(__as<__m128i>(__x), 31);
					const auto __invert = _mm_xor_si128(__as<__m128i>(__x), __sign);

					return __as<_Tp_>(_mm_sub_epi32(__invert, __sign));
				}
			}
			else if constexpr (__is_epi16_v<_Type_>) {
				if constexpr (__has_ssse3_support_v<_ISA_>) return __as<_Tp_>(_mm_abs_epi16(__as<__m128i>(__x)));
				else {
					const auto __negate = _mm_sub_epi16(_mm_setzero_si128(), __as<__m128i>(__x));
					return _mm_max_epi16(__as<__m128i>(__x), __negate);
				}
			}
			else if constexpr (__is_epi8_v<_Type_>) {
				if constexpr (__has_ssse3_support_v<_ISA_>) return __as<_Tp_>(_mm_abs_epi8(__as<__m128i>(__x)));
				else {
					const auto __negate = _mm_sub_epi8(_mm_setzero_si128(), __as<__m128i>(__x));
					return __as<_Tp_>(_mm_min_epu8(__as<__m128i>(__x), __negate));
				}
			}
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_and_ps(__as<__m128>(__x), __as<__m128>(_mm_set1_epi32(0x7FFFFFFF))));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_and_pd(__as<__m128d>(__x), __as<__m128d>(_mm_setr_epi32(-1, 0x7FFFFFFFu, -1, 0x7FFFFFFFu))));
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_and_pd(__as<__m256d>(__x), __as<__m256d>(_mm256_setr_epi32(
				0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu))));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_and_ps(__as<__m256>(__x), __as<__m256>(_mm256_set1_epi32(0x7FFFFFFFu))));
			else if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_>) {
					if constexpr (__has_avx512vl_support_v<_ISA_>) return __as<_Tp_>(_mm256_abs_epi64(__as<__m256i>(__x)));
					else {
						const auto __sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), __x);
						const auto __inverted = _mm256_xor_si256(__as<__m256i>(__x), __sign);

						return __as<_Tp_>(_mm256_sub_epi64(__inverted, __sign));
					}
				}
				else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_abs_epi32(__as<__m256i>(__x)));
				else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_abs_epi16(__as<__m256i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_abs_epi8(__as<__m256i>(__x)));
			}
			else {
				const auto __low = _Abs<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x));
				const auto __high = _Abs<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1));

				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_abs_epi64(__as<__m512i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_abs_epi32(__as<__m512i>(__x)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_abs_ps(__as<__m512>(__x)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_abs_pd(__as<__m512d>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_abs_epi16(__as<__m512i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_abs_epi8(__as<__m512i>(__x)));
			}
			else {
				const auto __low = __as<__m256i>(__x);
				const auto __high = _mm512_extractf64x4_pd(__as<__m512d>(__x), 1);

				auto __result = __as<__m512i>(_Abs<arch::ISA::AVX2, _Type_>()(__low));
				__result = __as<__m512i>(_mm512_insertf64x4(__as<__m512d>(__result),
					__as<__m256d>(_Abs<arch::ISA::AVX2, _Type_>()(__high)), 1));

				return __as<_Tp_>(__result);
			}
		}

		return __x < 0 ? -__x : __x;
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
		if constexpr (std::is_unsigned_v<_Type_>)
			return _Selectz<_ISA_, _Type_>()(__x, __mask);
		
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_maskz_abs_epi64(__mask, __as<__m128i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_maskz_abs_epi32(__mask, __as<__m128i>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_maskz_abs_epi16(__mask, __as<__m128i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_maskz_abs_epi8(__mask, __as<__m128i>(__x)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_abs_epi64(__mask, __as<__m256i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_abs_epi32(__mask, __as<__m256i>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_abs_epi16(__mask, __as<__m256i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_abs_epi8(__mask, __as<__m256i>(__x)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_epi64(__mask, __as<__m512i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_epi32(__mask, __as<__m512i>(__x)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_ps(__mask, __as<__m512>(__x)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_pd(__mask, __as<__m512d>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_epi16(__mask, __as<__m512i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_abs_epi8(__mask, __as<__m512i>(__x)));
			}
		}
		
		if constexpr (arithmetic_type<_Tp_>) return (__mask) ? (__x < 0 ? -__x : __x) : 0;
		else return _Selectz<_ISA_, _Type_>()((*this)(__x), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (std::is_unsigned_v<_Type_>)
			return _Select<_ISA_, _Type_>()(__x, __src, __mask);

		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm_mask_abs_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm_mask_abs_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_mask_abs_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_mask_abs_epi8(__as<__m128i>(__src), __mask, __as<__m128i>(__x)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_mask_abs_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_mask_abs_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_mask_abs_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_mask_abs_epi8(__as<__m256i>(__src), __mask, __as<__m256i>(__x)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x)));
			else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_ps(__as<__m512>(__src), __mask, __as<__m512>(__x)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_pd(__as<__m512d>(__src), __mask, __as<__m512d>(__x)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x)));
				else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm512_mask_abs_epi8(__as<__m512i>(__src), __mask, __as<__m512i>(__x)));
			}
		}
		
		if constexpr (arithmetic_type<_Tp_>) return (__mask) ? (__x < 0 ? -__x : __x) : __src;
		else return _Select<_ISA_, _Type_>()((*this)(__x), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END