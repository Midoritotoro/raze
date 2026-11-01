#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/compare/Less.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto less_equal_(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_pd_mask(x, y, _CMP_LE_OQ);
            else return _mm_cmple_pd(x, y);
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm_cmp_ps_mask(x, y, _CMP_LE_OQ);
            else return _mm_cmple_ps(x, y);
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (epi64<T>) return _mm_cmple_epi64_mask(x, y);
            else if constexpr (epu64<T>) return _mm_cmple_epu64_mask(x, y);
            else if constexpr (epi32<T>) return _mm_cmple_epi32_mask(x, y);
            else if constexpr (epu32<T>) return _mm_cmple_epu32_mask(x, y);
            else if constexpr (has_avx512bw<ISA>) {
                if constexpr (epi16<T>) return _mm_cmple_epi16_mask(x, y);
                else if constexpr (epu16<T>) return _mm_cmple_epu16_mask(x, y);
                else if constexpr (epi8<T>) return _mm_cmple_epi8_mask(x, y);
                else if constexpr (epu8<T>) return _mm_cmple_epu8_mask(x, y);
            }
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (pd<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_pd_mask(x, y, _CMP_LE_OQ);
            else return _mm256_cmp_pd(x, y, _CMP_LE_OQ);
        }
        else if constexpr (ps<T>) {
            if constexpr (has_avx512vl<ISA>) return _mm256_cmp_ps_mask(x, y, _CMP_LE_OQ);
            else return _mm256_cmp_ps(x, y, _CMP_LE_OQ);
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (epi64<T>) return _mm256_cmple_epi64_mask(x, y);
            else if constexpr (epu64<T>) return _mm256_cmple_epu64_mask(x, y);
            else if constexpr (epi32<T>) return _mm256_cmple_epi32_mask(x, y);
            else if constexpr (epu32<T>) return _mm256_cmple_epu32_mask(x, y);
            else if constexpr (has_avx512bw<ISA>) {
                if constexpr (epi16<T>) return _mm256_cmple_epi16_mask(x, y);
                else if constexpr (epu16<T>) return _mm256_cmple_epu16_mask(x, y);
                else if constexpr (epi8<T>) return _mm256_cmple_epi8_mask(x, y);
                else if constexpr (epu8<T>) return _mm256_cmple_epu8_mask(x, y);
            }
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return _mm512_cmple_pd_mask(x, y);
        else if constexpr (ps<T>) return _mm512_cmple_ps_mask(x, y);
        else if constexpr (epi64<T>) return _mm512_cmple_epi64_mask(x, y);
        else if constexpr (epu64<T>) return _mm512_cmple_epu64_mask(x, y);
        else if constexpr (epi32<T>) return _mm512_cmple_epi32_mask(x, y);
        else if constexpr (epu32<T>) return _mm512_cmple_epu32_mask(x, y);
        else if constexpr (has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm512_cmple_epi16_mask(x, y);
            else if constexpr (epu16<T>) return _mm512_cmple_epu16_mask(x, y);
            else if constexpr (epi8<T>) return _mm512_cmple_epi8_mask(x, y);
            else if constexpr (epu8<T>) return _mm512_cmple_epu8_mask(x, y);
        }
    }
    
    if constexpr (arithmetic_type<V>) return x <= y;
    else return bit_not_<ISA, T>(less_<ISA, T>(y, x));
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_always_inline auto less_equal_(V x, V y, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
        if constexpr (epi64<T>) return _mm_mask_cmple_epi64_mask(mask, x, y);
        else if constexpr (epu64<T>) return _mm_mask_cmple_epu64_mask(mask, x, y);
        else if constexpr (epi32<T>) return _mm_mask_cmple_epi32_mask(mask, x, y);
        else if constexpr (epu32<T>) return _mm_mask_cmple_epu32_mask(mask, x, y);
        else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm_mask_cmple_epi16_mask(mask, x, y);
            else return _mm_mask_cmple_epu16_mask(mask, x, y);
        }
        else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA>) {
            if constexpr (epi8<T>) return _mm_mask_cmple_epi8_mask(mask, x, y);
            else return _mm_mask_cmple_epu8_mask(mask, x, y);
        }
        else if constexpr (pd<T>) return _mm_mask_cmp_pd_mask(mask, x, y, _CMP_LE_OQ);
        else if constexpr (ps<T>) return _mm_mask_cmp_ps_mask(mask, x, y, _CMP_LE_OQ);
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
        if constexpr (epi64<T>) return _mm256_mask_cmple_epi64_mask(mask, x, y);
        else if constexpr (epu64<T>) return _mm256_mask_cmple_epu64_mask(mask, x, y);
        else if constexpr (epi32<T>) return _mm256_mask_cmple_epi32_mask(mask, x, y);
        else if constexpr (epu32<T>) return _mm256_mask_cmple_epu32_mask(mask, x, y);
        else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm256_mask_cmple_epi16_mask(mask, x, y);
            else return _mm256_mask_cmple_epu16_mask(mask, x, y);
        }
        else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA>) {
            if constexpr (epi8<T>) return _mm256_mask_cmple_epi8_mask(mask, x, y);
            else return _mm256_mask_cmple_epu8_mask(mask, x, y);
        }
        else if constexpr (pd<T>) return _mm256_mask_cmp_pd_mask(mask, x, y, _CMP_LE_OQ);
        else if constexpr (ps<T>) return _mm256_mask_cmp_ps_mask(mask, x, y, _CMP_LE_OQ);
    }
    else if constexpr (sizeof(V) == 64 && has_avx512f<ISA>) {
        if constexpr (epi64<T>) return _mm512_mask_cmple_epi64_mask(mask, x, y);
        else if constexpr (epu64<T>) return _mm512_mask_cmple_epu64_mask(mask, x, y);
        else if constexpr (epi32<T>) return _mm512_mask_cmple_epi32_mask(mask, x, y);
        else if constexpr (epu32<T>) return _mm512_mask_cmple_epu32_mask(mask, x, y);
        else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA>) {
            if constexpr (epi16<T>) return _mm512_mask_cmple_epi16_mask(mask, x, y);
            else return _mm512_mask_cmple_epu16_mask(mask, x, y);
        }
        else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA>) {
            if constexpr (epi8<T>) return _mm512_mask_cmple_epi8_mask(mask, x, y);
            else return _mm512_mask_cmple_epu8_mask(mask, x, y);
        }
        else if constexpr (pd<T>) return _mm512_mask_cmple_pd_mask(mask, x, y);
        else if constexpr (ps<T>) return _mm512_mask_cmple_ps_mask(mask, x, y);
    }

    return mask_and_<ISA, T>(less_equal_<ISA, T>(x, y), mask);
}

__RAZE_VX_NAMESPACE_END
