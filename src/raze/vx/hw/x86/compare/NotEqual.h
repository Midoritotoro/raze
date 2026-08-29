#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto operator()(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_pd_mask(as<__m128d>(x), as<__m128d>(y), _CMP_NEQ_OQ);
            else return as<V>(_mm_cmpneq_pd(as<__m128d>(x), as<__m128d>(y)));
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_ps_mask(as<__m128>(x), as<__m128>(y), _CMP_NEQ_OQ);
            else return as<V>(_mm_cmpneq_ps(as<__m128>(x), as<__m128>(y)));
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (epi64<T> || epu64<T>) return _mm_cmpneq_epi64_mask(as<__m128i>(x), as<__m128i>(y));
            else if constexpr (epi32<T> || epu32<T>) return _mm_cmpneq_epi32_mask(as<__m128i>(x), as<__m128i>(y));
            else if constexpr (has_avx512bw<ISA>) {
                if constexpr (epi16<T>) return _mm_cmpneq_epi16_mask(as<__m128i>(x), as<__m128i>(y));
                else if constexpr (epu16<T>) return _mm_cmpneq_epu16_mask(as<__m128i>(x), as<__m128i>(y));
                else if constexpr (epi8<T>) return _mm_cmpneq_epi8_mask(as<__m128i>(x), as<__m128i>(y));
                else if constexpr (epu8<T>) return _mm_cmpneq_epu8_mask(as<__m128i>(x), as<__m128i>(y));
            }
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_pd_mask(as<__m256d>(x), as<__m256d>(y), _CMP_NEQ_OQ);
            else return as<V>(_mm256_cmp_pd(as<__m256d>(x), as<__m256d>(y), _CMP_NEQ_OQ));
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_ps_mask(as<__m256>(x), as<__m256>(y), _CMP_NEQ_OQ);
            else return as<V>(_mm256_cmp_ps(as<__m256>(x), as<__m256>(y), _CMP_NEQ_OQ));
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (epi64<T> || epu64<T>) return _mm256_cmpneq_epi64_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (epi32<T> || epu32<T>) return _mm256_cmpneq_epi32_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (has_avx512bw<ISA>) {
                if constexpr (epi16<T>) return _mm256_cmpneq_epi16_mask(as<__m256i>(x), as<__m256i>(y));
                else if constexpr (epu16<T>) return _mm256_cmpneq_epu16_mask(as<__m256i>(x), as<__m256i>(y));
                else if constexpr (epi8<T>) return _mm256_cmpneq_epi8_mask(as<__m256i>(x), as<__m256i>(y));
                else if constexpr (epu8<T>) return _mm256_cmpneq_epu8_mask(as<__m256i>(x), as<__m256i>(y));
            }
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return _mm512_cmpneq_pd_mask(as<__m512d>(x), as<__m512d>(y));
        else if constexpr (ps<T>) return _mm512_cmpneq_ps_mask(as<__m512>(x), as<__m512>(y));
        else if constexpr (epi64<T> || epu64<T>) return _mm512_cmpneq_epi64_mask(as<__m512i>(x), as<__m512i>(y));
        else if constexpr (epi32<T> || epu32<T>) return _mm512_cmpneq_epi32_mask(as<__m512i>(x), as<__m512i>(y));
        else if constexpr (has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm512_cmpneq_epi16_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epu16<T>) return _mm512_cmpneq_epu16_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epi8<T>) return _mm512_cmpneq_epi8_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epu8<T>) return _mm512_cmpneq_epu8_mask(as<__m512i>(x), as<__m512i>(y));
        }
    }

    if constexpr (arithmetic_type<V>) return x != y;
    else return bit_not_<ISA, T>(equal_<ISA, T>(x, y));
}

__RAZE_VX_NAMESPACE_END
