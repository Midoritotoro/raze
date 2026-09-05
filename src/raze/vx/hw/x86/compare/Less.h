#pragma once 

#include <src/raze/vx/hw/x86/bitwise/BitNot.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto less_(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_pd_mask(x, y, _CMP_LT_OQ);
            else return _mm_cmplt_pd(x, y);
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_ps_mask(x, y, _CMP_LT_OQ);
            else return _mm_cmplt_ps(x, y);
        }
        else if constexpr (epi64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmplt_epi64_mask(x, y);
            else if constexpr (has_sse42<ISA>) return _mm_cmpgt_epi64(y, x);
            else {
                const auto difference_andnot_xor = _mm_andnot_si128(_mm_xor_si128(x, y), _mm_sub_epi64(x, y));
                const auto combined = _mm_or_si128(_mm_andnot_si128(y, x), difference_andnot_xor);
                return _mm_shuffle_epi32(_mm_srai_epi32(combined, 31), 0xF5);
            }
        }
        else if constexpr (epu64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmplt_epu64_mask(x, y);
            else if constexpr (has_sse42<ISA>) {
                const auto sign = _mm_set1_epi64x(0x8000000000000000);
                return _mm_cmpgt_epi64(_mm_xor_si128(y, sign), _mm_xor_si128(x, sign));
            }
            else {
                const auto sign = _mm_set1_epi32(0x80000000);

                const auto greater = _mm_cmplt_epi32(_mm_xor_si128(x, sign), _mm_xor_si128(y, sign));
                const auto equal_greater = _mm_and_si128(_mm_cmpeq_epi32(x, y), _mm_shuffle_epi32(greater, 0xA0));

                return _mm_shuffle_epi32(_mm_or_si128(greater, equal_greater), 0xF5);
            }
        }
        else if constexpr (epi32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmplt_epi32_mask(x, y);
            else return _mm_cmplt_epi32(x, y);
        }
        else if constexpr (epu32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmplt_epu32_mask(x, y);
            else {
                const auto sign = _mm_set1_epi32(0x80000000);
                return _mm_cmplt_epi32(_mm_xor_si128(x, sign), _mm_xor_si128(y, sign));
            }
        }
        else if constexpr (epi16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmplt_epi16_mask(x, y);
            else return _mm_cmplt_epi16(x, y);
        }
        else if constexpr (epu16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmplt_epu16_mask(x, y);
            else return bit_not_<arch::ISA::SSE2, T>(_mm_cmpeq_epi16(_mm_subs_epu16(y, x), _mm_setzero_si128()));
        }
        else if constexpr (epi8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmplt_epi8_mask(x, y);
            else return _mm_cmplt_epi8(x, y);
        }
        else if constexpr (epu8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmplt_epu8_mask(x, y);
            else return bit_not_<arch::ISA::SSE2, T>(_mm_cmpeq_epi8(_mm_subs_epu8(y, x), _mm_setzero_si128()));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        auto fallback_avx = [=] (auto x, auto y) raze_always_inline_lambda {
            const auto low = less_<arch::ISA::SSE42, T>(as<__m128i>(x), as<__m128i>(y));
            const auto high = less_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1),
                _mm256_extractf128_si256(as<__m256i>(y), 1));

            return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
        };

        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_pd_mask(x, y, _CMP_LT_OQ);
            else return _mm256_cmp_pd(x, y, _CMP_LT_OQ);
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_ps_mask(x, y, _CMP_LT_OQ);
            else return _mm256_cmp_ps(x, y, _CMP_LT_OQ);
        }
        else if constexpr (epi64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmplt_epi64_mask(x, y);
            else if constexpr (has_avx2<ISA>) return _mm256_cmpgt_epi64(y, x);
            else return fallback_avx(x, y);
        }
        else if constexpr (epu64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmplt_epu64_mask(x, y);
            else if constexpr (has_avx2<ISA>) {
                const auto sign = _mm256_set1_epi64x(0x8000000000000000);
                return _mm256_cmpgt_epi64(_mm256_xor_si256(y, sign), _mm256_xor_si256(x, sign));
            }
            else return fallback_avx(x, y);
        }
        else if constexpr (epi32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmplt_epi32_mask(x, y);
            else if constexpr (has_avx2<ISA>) return _mm256_cmpgt_epi32(y, x);
            else return fallback_avx(x, y);
        }
        else if constexpr (epu32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmplt_epu32_mask(x, y);
            else if constexpr (has_avx2<ISA>) {
                const auto sign = _mm256_set1_epi32(0x80000000);
                return _mm256_cmpgt_epi32(_mm256_xor_si256(y, sign), _mm256_xor_si256(x, sign));
            }
            else return fallback_avx(x, y);
        }
        else if constexpr (epi16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmplt_epi16_mask(x, y);
            else if constexpr (has_avx2<ISA>) return _mm256_cmpgt_epi16(y, x);
            else return fallback_avx(x, y);
        }
        else if constexpr (epu16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmplt_epu16_mask(x, y);
            else if constexpr (has_avx2<ISA>) {
                const auto sign = _mm256_set1_epi16(0x8000);
                return _mm256_cmpgt_epi16(_mm256_xor_si256(y, sign), _mm256_xor_si256(x, sign));
            }
            else return fallback_avx(x, y);
        }
        else if constexpr (epi8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmplt_epi8_mask(x, y);
            else if constexpr (has_avx2<ISA>) return _mm256_cmpgt_epi8(y, x);
            else return fallback_avx(x, y);
        }
        else if constexpr (epu8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmplt_epu8_mask(x, y);
            else if constexpr (has_avx2<ISA>) {
                const auto sign = _mm256_set1_epi8(0x80);
                return _mm256_cmpgt_epi8(_mm256_xor_si256(y, sign), _mm256_xor_si256(x, sign));
            }
            else return fallback_avx(x, y);
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (epi64<T>) return _mm512_cmplt_epi64_mask(x, y);
        else if constexpr (epu64<T>) return _mm512_cmplt_epu64_mask(x, y);
        else if constexpr (epi32<T>) return _mm512_cmplt_epi32_mask(x, y);
        else if constexpr (epu32<T>) return _mm512_cmplt_epu32_mask(x, y);
        else if constexpr (ps<T>) return _mm512_cmplt_ps_mask(x, y);
        else if constexpr (pd<T>) return _mm512_cmplt_pd_mask(x, y);
        else if constexpr (has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm512_cmplt_epi16_mask(x, y);
            else if constexpr (epu16<T>) return _mm512_cmplt_epu16_mask(x, y);
            else if constexpr (epi8<T>) return _mm512_cmplt_epi8_mask(x, y);
            else if constexpr (epu8<T>)  return _mm512_cmplt_epu8_mask(x, y);
        }
        else {
            const auto compared_low = less_<arch::ISA::AVX2, T>(as<__m256i>(x), as<__m256i>(y));
            const auto compared_high = less_<arch::ISA::AVX2, T>(_mm512_extracti64x4_epi64(as<__m512i>(x), 1),
                _mm512_extracti64x4_epi64(as<__m512i>(y), 1));

            return as<V>(_mm512_inserti64x4(as<__m512i>(compared_low), compared_high, 1));
        }
    }
    else {
        return x < y;
    }
}

__RAZE_VX_NAMESPACE_END
