#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Add {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_add_epi64(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_add_epi32(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_add_epi16(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_add_epi8(__as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_add_ps(__as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_add_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_add_ps(__as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_add_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_add_epi64(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_add_epi32(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_add_epi16(__as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_add_epi8(__as<__m256i>(__x), __as<__m256i>(__y)));
			}
			else {
				const auto __low = _Add<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __as<__m128i>(__y));
				const auto __high = _Add<arch::ISA::SSE42, _Type_>()(
					_mm256_extractf128_si256(__as<__m256i>(__x), 1), _mm256_extractf128_si256(__as<__m256i>(__y), 1));

				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_add_pd(__as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_add_ps(__as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_add_epi32(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_add_epi64(__as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_add_epi8(__as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_add_epi16(__as<__m512i>(__x), __as<__m512i>(__y)));
			}
			else {
				const auto __low = _Add<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x), __as<__m256i>(__y));
				const auto __high = _Add<arch::ISA::AVX2, _Type_>()(
					_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1), _mm512_extracti64x4_epi64(__as<__m512i>(__y), 1));

				return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
			}
		}
		else {
			return __x + __y;
		}
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_epi64(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_epi32(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_ps(__mask, __as<__m128>(__x), __as<__m128>(__y)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_pd(__mask, __as<__m128d>(__x), __as<__m128d>(__y)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_epi16(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_maskz_add_epi8(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_epi64(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_epi32(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_ps(__mask, __as<__m256>(__x), __as<__m256>(__y)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_pd(__mask, __as<__m256d>(__x), __as<__m256d>(__y)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_epi16(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_add_epi8(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_epi64(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_epi32(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_ps(__mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_pd(__mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_epi16(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_add_epi8(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}

		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_add_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_add_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_add_ps(__as<__m128>(__src), __mask, __as<__m128>(__x), __as<__m128>(__y)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_add_pd(__as<__m128d>(__src), __mask, __as<__m128d>(__x), __as<__m128d>(__y)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_add_epi16(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_mask_add_epi8(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__has_avx512vl_support_v<_ISA_>) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_ps(__as<__m256>(__src), __mask, __as<__m256>(__x), __as<__m256>(__y)));
				else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_pd(__as<__m256d>(__src), __mask, __as<__m256d>(__x), __as<__m256d>(__y)));
				else if constexpr (__has_avx512bw_support_v<_ISA_>) {
					if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_epi16(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
					else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_mask_add_epi8(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_ps(__as<__m512>(__src), __mask, __as<__m512>(__x), __as<__m512>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_pd(__as<__m512d>(__src), __mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__has_avx512bw_support_v<_ISA_>) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_epi16(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_mask_add_epi8(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			}
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
