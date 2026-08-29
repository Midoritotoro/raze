#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto equal_(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_pd_mask(as<__m128d>(x), as<__m128d>(y), _CMP_EQ_OQ);
            else return as<V>(_mm_cmpeq_pd(as<__m128d>(x), as<__m128d>(y)));
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_ps_mask(as<__m128>(x), as<__m128>(y), _CMP_EQ_OQ);
            else return as<V>(_mm_cmpeq_ps(as<__m128>(x), as<__m128>(y)));
        }
        else if constexpr (epi64<T> || epu64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmpeq_epi64_mask(as<__m128i>(x), as<__m128i>(y));
            else if constexpr (has_sse41<ISA>) return as<V>(_mm_cmpeq_epi64(as<__m128i>(x), as<__m128i>(y)));
            else {
                const auto equal_mask = _mm_cmpeq_epi32(as<__m128i>(x), as<__m128i>(y));
                return as<V>(_mm_and_si128(equal_mask, _mm_shuffle_epi32(equal_mask, 0xB1)));
            }
        }
        else if constexpr (epi32<T> || epu32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmpeq_epi32_mask(as<__m128i>(x), as<__m128i>(y));
            else return as<V>(_mm_cmpeq_epi32(as<__m128i>(x), as<__m128i>(y)));
        }
        else if constexpr (epi16<T> || epu16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmpeq_epi16_mask(as<__m128i>(x), as<__m128i>(y));
            else return as<V>(_mm_cmpeq_epi16(as<__m128i>(x), as<__m128i>(y)));
        }
        else if constexpr (epi8<T> || epu8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cmpeq_epi8_mask(as<__m128i>(x), as<__m128i>(y));
            else return as<V>(_mm_cmpeq_epi8(as<__m128i>(x), as<__m128i>(y)));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        auto fallback_avx = [=]() raze_always_inline_lambda {
            const auto low = equal_<arch::ISA::SSE42, T>(as<__m128i>(x), as<__m128i>(y));
            const auto high = equal_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1),
                _mm256_extractf128_si256(as<__m256i>(y), 1));

            return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
        };

        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_pd_mask(as<__m256d>(x), as<__m256d>(y), _CMP_EQ_OQ);
            else return as<V>(_mm256_cmp_pd(as<__m256d>(x), as<__m256d>(y), _CMP_EQ_OQ));
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_ps_mask(as<__m256>(x), as<__m256>(y), _CMP_EQ_OQ);
            else return as<V>(_mm256_cmp_ps(as<__m256>(x), as<__m256>(y), _CMP_EQ_OQ));
        }
        else if constexpr (epi64<T> || epu64<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmpeq_epi64_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (has_avx2<ISA>) return as<V>(_mm256_cmpeq_epi64(as<__m256i>(x), as<__m256i>(y)));
            else return fallback_avx();
        }
        else if constexpr (epi32<T> || epu32<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmpeq_epi32_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (has_avx2<ISA>) return as<V>(_mm256_cmpeq_epi32(as<__m256i>(x), as<__m256i>(y)));
            else return fallback_avx();
        }
        else if constexpr (epi16<T> || epu16<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmpeq_epi16_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (has_avx2<ISA>) return as<V>(_mm256_cmpeq_epi16(as<__m256i>(x), as<__m256i>(y)));
            else return fallback_avx();
        }
        else if constexpr (epi8<T> || epu8<T>) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cmpeq_epi8_mask(as<__m256i>(x), as<__m256i>(y));
            else if constexpr (has_avx2<ISA>) return as<V>(_mm256_cmpeq_epi8(as<__m256i>(x), as<__m256i>(y)));
            else return fallback_avx();
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return _mm512_cmpeq_pd_mask(as<__m512d>(x), as<__m512d>(y));
        else if constexpr (ps<T>) return _mm512_cmpeq_ps_mask(as<__m512>(x), as<__m512>(y));
        else if constexpr (epi64<T> || epu64<T>) return _mm512_cmpeq_epi64_mask(as<__m512i>(x), as<__m512i>(y));
        else if constexpr (epi32<T> || epu32<T>) return _mm512_cmpeq_epi32_mask(as<__m512i>(x), as<__m512i>(y));
        else if constexpr (has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm512_cmpeq_epi16_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epu16<T>) return _mm512_cmpeq_epu16_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epi8<T>) return _mm512_cmpeq_epi8_mask(as<__m512i>(x), as<__m512i>(y));
            else if constexpr (epu8<T>) return _mm512_cmpeq_epu8_mask(as<__m512i>(x), as<__m512i>(y));
        }
        else {
            const auto low = equal_<arch::ISA::AVX2, T>(as<__m256i>(x), as<__m256i>(y));
            const auto high = equal_<arch::ISA::AVX2, T>(_mm512_extracti64x4_epi64(as<__m512i>(x), 1),
                _mm512_extracti64x4_epi64(as<__m512i>(y), 1));

            return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
        }
    }
    else {
        return x == y;
    }
}

__RAZE_VX_NAMESPACE_END
