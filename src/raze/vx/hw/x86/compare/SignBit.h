#pragma once

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <raze/math/BitCast.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto signbit_(V x) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (has_avx512vl<ISA>) {
            if constexpr (has_avx512dq<ISA>) {
                if constexpr (pd<T>) return _mm_movepi64_mask(as<__m128i>(x));
                else if constexpr (ps<T>) return _mm_movepi32_mask(as<__m128i>(x));
            }
            else {
                if constexpr (pd<T>) return _mm_cmplt_epi64_mask(as<__m128i>(x), _mm_setzero_si128());
                else if constexpr (ps<T>) return _mm_cmplt_epi32_mask(as<__m128i>(x), _mm_setzero_si128());
            }
        }
        else {
            if constexpr (pd<T>) return _mm_shuffle_epi32(_mm_srai_epi32(as<__m128i>(x), 31), 0xF5);
            else if constexpr (ps<T>) return _mm_srai_epi32(as<__m128i>(x), 31);
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (has_avx512vl<ISA>) {
            if constexpr (has_avx512dq<ISA>) {
                if constexpr (pd<T>) return _mm256_movepi64_mask(as<__m256i>(x));
                else if constexpr (ps<T>) return _mm256_movepi32_mask(as<__m256i>(x));
            }
            else {
                if constexpr (pd<T>) return _mm256_cmplt_epi64_mask(as<__m256i>(x), _mm256_setzero_si256());
                else if constexpr (ps<T>) return _mm256_cmplt_epi32_mask(as<__m256i>(x), _mm256_setzero_si256());
            }
        }
        else if constexpr (has_avx2<ISA>) {
            if constexpr (pd<T>) return _mm256_shuffle_epi32(_mm256_srai_epi32(as<__m256i>(x), 31), 0xF5);
            else if constexpr (ps<T>) return _mm256_srai_epi32(as<__m256i>(x), 31);
        }
        else {
            const auto lo = signbit_<arch::ISA::SSE42, T>(as<__m128i>(x));
            const auto hi = signbit_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1));
            return _mm256_insertf128_si256(as<__m256i>(lo), hi, 1);
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm512_movepi64_mask(as<__m512i>(x));
            else if constexpr (ps<T>) return _mm512_movepi32_mask(as<__m512i>(x));
        }
        else {
            if constexpr (pd<T>) return _mm512_cmplt_epi64_mask(as<__m512i>(x), _mm512_setzero_si512());
            else if constexpr (ps<T>) return _mm512_cmplt_epi32_mask(as<__m512i>(x), _mm512_setzero_si512());
        }
    }
    else {
        if constexpr (pd<T>) return (math::bit_cast<u64>(x) >> 63) != 0ull;
        else return (math::bit_cast<u32>(x) >> 31) != 0u;
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto signbit_(V x, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && !has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm_mask_cmplt_epi64_mask(mask, as<__m128i>(x), _mm_setzero_si128());
        else if constexpr (ps<T>) return _mm_mask_cmplt_epi32_mask(mask, as<__m128i>(x), _mm_setzero_si128());
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && !has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm256_mask_cmplt_epi64_mask(mask, as<__m256i>(x), _mm256_setzero_si256());
        else if constexpr (ps<T>) return _mm256_mask_cmplt_epi32_mask(mask, as<__m256i>(x), _mm256_setzero_si256());
    }
    else if constexpr (sizeof(V) == 64 && has_avx512f<ISA> && !has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm512_mask_cmplt_epi64_mask(mask, as<__m512i>(x), _mm512_setzero_si512());
        else if constexpr (ps<T>) return _mm512_mask_cmplt_epi32_mask(mask, as<__m512i>(x), _mm512_setzero_si512());
    }

    return mask_and_<ISA, T>(signbit_<ISA, T>(x), mask);
}

__RAZE_VX_NAMESPACE_END

