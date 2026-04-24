#pragma once 

#include <src/raze/vx/hw/x86/mask/ToMask.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	    _ISA_,
	arithmetic_type	_Type_>
struct _Less {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (__has_avx512vl_support_v<_ISA_>) {
                if constexpr (__is_pd_v<_Type_>) return _mm_cmp_pd_mask(__as<__m128d>(__x), __as<__m128d>(__y), _CMP_LT_OQ);
                else if constexpr (__is_ps_v<_Type_>) return _mm_cmp_ps_mask(__as<__m128>(__x), __as<__m128>(__y), _CMP_LT_OQ);
                else if constexpr (__is_epi64_v<_Type_>) return _mm_cmplt_epi64_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__is_epu64_v<_Type_>) return _mm_cmplt_epu64_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__is_epi32_v<_Type_>) return _mm_cmplt_epi32_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__is_epu32_v<_Type_>) return _mm_cmplt_epu32_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                    if constexpr (__is_epi16_v<_Type_>) return _mm_cmplt_epi16_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epu16_v<_Type_>) return _mm_cmplt_epu16_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epi8_v<_Type_>) return _mm_cmplt_epi8_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epu8_v<_Type_>) return _mm_cmplt_epu8_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                }
            }
            else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_cmplt_ps(__as<__m128>(__x), __as<__m128>(__y)));
            else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_cmplt_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
            else if constexpr (__is_epi64_v<_Type_>) {
                if constexpr (__has_sse41_support_v<_ISA_>) {
                    return __as<_Tp_>(_mm_cmpgt_epi64(__as<__m128i>(__y), __as<__m128i>(__x)));
                }
                else {
                    const auto __difference64 = _mm_sub_epi64(__as<__m128i>(__x), __as<__m128i>(__y));

                    const auto __xor_mask = _mm_xor_si128(__as<__m128i>(__x), __as<__m128i>(__y));
                    const auto __left_andnot_right = _mm_andnot_si128(__as<__m128i>(__y), __as<__m128i>(__x));
                    const auto __difference_andnot_xor = _mm_andnot_si128(__xor_mask, __difference64);

                    const auto __combined_mask = _mm_or_si128(__left_andnot_right, __difference_andnot_xor);
                    return __as<_Tp_>(_mm_shuffle_epi32(_mm_srai_epi32(__combined_mask, 31), 0xF5));
                }
            }
            else if constexpr (__is_epu64_v<_Type_>) {
                if constexpr (__has_sse41_support_v<_ISA_>) {
                    const auto __sign_64bit = _mm_set1_epi64x(0x8000000000000000);

                    const auto __left_signed = _mm_xor_si128(__as<__m128i>(__x), __sign_64bit);
                    const auto __right_signed = _mm_xor_si128(__as<__m128i>(__y), __sign_64bit);

                    return __as<_Tp_>(_mm_cmpgt_epi64(__right_signed, __left_signed));
                }
                else {
                    const auto __32bit_sign = _mm_set1_epi32(0x80000000);

                    const auto __signed_32bit_left = _mm_xor_si128(__as<__m128i>(__x), __32bit_sign);
                    const auto __signed_32bit_right = _mm_xor_si128(__as<__m128i>(__y), __32bit_sign);

                    const auto __equal = _mm_cmpeq_epi32(__as<__m128i>(__x), __as<__m128i>(__y));
                    const auto __bigger = _mm_cmplt_epi32(__signed_32bit_left, __signed_32bit_right);

                    const auto __shuffled_bigger = _mm_shuffle_epi32(__bigger, 0xA0);
                    const auto __equal_bigger = _mm_and_si128(__equal, __shuffled_bigger);

                    return __as<_Tp_>(_mm_shuffle_epi32(_mm_or_si128(__bigger, __equal_bigger), 0xF5));
                }
            }
            else if constexpr (__is_epi32_v<_Type_>)  return __as<_Tp_>(_mm_cmplt_epi32(__as<__m128i>(__x), __as<__m128i>(__y)));
            else if constexpr (__is_epu32_v<_Type_>) {
                const auto __sign = _mm_set1_epi32(0x80000000);

                const auto __signed_left = _mm_xor_si128(__as<__m128i>(__x), __sign);
                const auto __signed_right = _mm_xor_si128(__as<__m128i>(__y), __sign);

                return __as<_Tp_>(_mm_cmplt_epi32(__signed_left, __signed_right));
            }
            else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm_cmplt_epi16(__as<__m128i>(__x), __as<__m128i>(__y)));
            else if constexpr (__is_epu16_v<_Type_>) {
                const auto __substracted = _mm_subs_epu16(__as<__m128i>(__y), __as<__m128i>(__x));
                return _Not<arch::ISA::SSE2, _Type_>()(__as<_Tp_>(_mm_cmpeq_epi16(__substracted, _mm_setzero_si128())));
            }
            else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm_cmplt_epi8(__as<__m128i>(__x), __as<__m128i>(__y)));
            else if constexpr (__is_epu8_v<_Type_>) {
                const auto __substracted = _mm_subs_epu8(__as<__m128i>(__y), __as<__m128i>(__x));
                return _Not<arch::ISA::SSE2, _Type_>()(__as<_Tp_>(_mm_cmpeq_epi8(__substracted, _mm_setzero_si128())));
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            if constexpr (__has_avx512vl_support_v<_ISA_>) {
                if constexpr (__is_pd_v<_Type_>) return _mm256_cmp_pd_mask(__as<__m256d>(__x), __as<__m256d>(__y), _CMP_LT_OQ);
                else if constexpr (__is_ps_v<_Type_>) return _mm256_cmp_ps_mask(__as<__m256>(__x), __as<__m256>(__y), _CMP_LT_OQ);
                else if constexpr (__is_epi64_v<_Type_>) return _mm256_cmplt_epi64_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__is_epu64_v<_Type_>) return _mm256_cmplt_epu64_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__is_epi32_v<_Type_>) return _mm256_cmplt_epi32_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__is_epu32_v<_Type_>) return _mm256_cmplt_epu32_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                    if constexpr (__is_epi16_v<_Type_>) return _mm256_cmplt_epi16_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epu16_v<_Type_>) return _mm256_cmplt_epu16_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epi8_v<_Type_>) return _mm256_cmplt_epi8_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epu8_v<_Type_>) return _mm256_cmplt_epu8_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                }
            }
            else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_cmp_ps(__as<__m256>(__x), __as<__m256>(__y), _CMP_LT_OQ));
            else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_cmp_pd(__as<__m256d>(__x), __as<__m256d>(__y), _CMP_LT_OQ));
            else if constexpr (__has_avx2_support_v<_ISA_>) {
                if constexpr (__is_epi64_v<_Type_>) return __as<_Tp_>(_mm256_cmpgt_epi64(__as<__m256i>(__y), __as<__m256i>(__x)));
                else if constexpr (__is_epu64_v<_Type_>) {
                    const auto __sign_64bit = _mm256_set1_epi64x(0x8000000000000000);

                    const auto __left_signed  = _mm256_xor_si256(__as<__m256i>(__x), __sign_64bit);
                    const auto __right_signed = _mm256_xor_si256(__as<__m256i>(__y), __sign_64bit);

                    return __as<_Tp_>(_mm256_cmpgt_epi64(__right_signed, __left_signed));
                }
                else if constexpr (__is_epi32_v<_Type_>) return __as<_Tp_>(_mm256_cmpgt_epi32(__as<__m256i>(__y), __as<__m256i>(__x)));
                else if constexpr (__is_epu32_v<_Type_>) {
                    const auto __sign_32bit = _mm256_set1_epi32(0x80000000);

                    const auto __left_signed = _mm256_xor_si256(__as<__m256i>(__x), __sign_32bit);
                    const auto __right_signed = _mm256_xor_si256(__as<__m256i>(__y), __sign_32bit);

                    return __as<_Tp_>(_mm256_cmpgt_epi32(__right_signed, __left_signed));
                }
                else if constexpr (__is_epi16_v<_Type_>) return __as<_Tp_>(_mm256_cmpgt_epi16(__as<__m256i>(__y), __as<__m256i>(__x)));
                else if constexpr (__is_epu16_v<_Type_>) {
                    const auto __sign_16bit = _mm256_set1_epi16(0x8000);

                    const auto __left_signed    = _mm256_xor_si256(__as<__m256i>(__x), __sign_16bit);
                    const auto __right_signed   = _mm256_xor_si256(__as<__m256i>(__y), __sign_16bit);

                    return __as<_Tp_>(_mm256_cmpgt_epi16(__right_signed, __left_signed));
                }
                else if constexpr (__is_epi8_v<_Type_>) return __as<_Tp_>(_mm256_cmpgt_epi8(__as<__m256i>(__y), __as<__m256i>(__x)));
                else if constexpr (__is_epu8_v<_Type_>) {
                    const auto __sign_8bit   = _mm256_set1_epi8(0x80);

                    const auto __left_signed    = _mm256_xor_si256(__as<__m256i>(__x), __sign_8bit);
                    const auto __right_signed   = _mm256_xor_si256(__as<__m256i>(__y), __sign_8bit);

                    return __as<_Tp_>(_mm256_cmpgt_epi8(__right_signed, __left_signed));
                }
            }
            else {
                const auto __low = _Less<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __as<__m128i>(__y));
                const auto __high = _Less<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1),
                    _mm256_extractf128_si256(__as<__m256i>(__y), 1));

                return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (__is_epi64_v<_Type_>) return _mm512_cmplt_epi64_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_epu64_v<_Type_>) return _mm512_cmplt_epu64_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_epi32_v<_Type_>) return _mm512_cmplt_epi32_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_epu32_v<_Type_>) return _mm512_cmplt_epu32_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_ps_v<_Type_>) return _mm512_cmplt_ps_mask(__as<__m512>(__x), __as<__m512>(__y));
            else if constexpr (__is_pd_v<_Type_>) return _mm512_cmplt_pd_mask(__as<__m512d>(__x), __as<__m512d>(__y));
            else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                if constexpr (__is_epi16_v<_Type_>) return _mm512_cmplt_epi16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu16_v<_Type_>) return _mm512_cmplt_epu16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epi8_v<_Type_>) return _mm512_cmplt_epi8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu8_v<_Type_>)  return _mm512_cmplt_epu8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            }
            else {
                const auto __compared_low = _Less<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x), __as<__m256i>(__y));
                const auto __compared_high = _Less<arch::ISA::AVX2, _Type_>()(_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1),
                    _mm512_extracti64x4_epi64(__as<__m512i>(__y), 1));

                return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__compared_low), __compared_high, 1));
            }
        }
        else {
            return __x < __y;
        }
	}
};

__RAZE_VX_NAMESPACE_END
