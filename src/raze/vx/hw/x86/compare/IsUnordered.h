#pragma once

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/compare/IsNan.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto is_unordered_(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (has_avx512vl<ISA>) {
            if constexpr (pd<T>) return _mm_cmp_pd_mask(as<__m128d>(x), as<__m128d>(y), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm_cmp_ps_mask(as<__m128>(x), as<__m128>(y), _CMP_UNORD_Q);
        }
        else {
            if constexpr (pd<T>) return _mm_cmpunord_pd(as<__m128d>(x), as<__m128d>(y));
            else if constexpr (ps<T>) return _mm_cmpunord_ps(as<__m128>(x), as<__m128>(y));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (has_avx512vl<ISA>) {
            if constexpr (pd<T>) return _mm256_cmp_pd_mask(as<__m256d>(x), as<__m256d>(y), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm256_cmp_ps_mask(as<__m256>(x), as<__m256>(y), _CMP_UNORD_Q);
        }
        else {
            if constexpr (pd<T>) return _mm256_cmp_pd(as<__m256d>(x), as<__m256d>(y), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm256_cmp_ps(as<__m256>(x), as<__m256>(y), _CMP_UNORD_Q);
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return _mm512_cmp_pd_mask(as<__m512d>(x), as<__m512d>(y), _CMP_UNORD_Q);
        else if constexpr (ps<T>) return _mm512_cmp_ps_mask(as<__m512>(x), as<__m512>(y), _CMP_UNORD_Q);
    }
    else {
        return is_nan_<ISA, T>(x) || is_nan_<ISA, T>(y);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto is_unordered_(V x, V y, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
        if constexpr (pd<T>) return _mm_mask_cmp_pd_mask(mask, as<__m128d>(x), as<__m128d>(y), _CMP_UNORD_Q);
        else if constexpr (ps<T>) return _mm_mask_cmp_ps_mask(mask, as<__m128>(x), as<__m128>(y), _CMP_UNORD_Q);
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
        if constexpr (pd<T>) return _mm256_mask_cmp_pd_mask(mask, as<__m256d>(x), as<__m256d>(y), _CMP_UNORD_Q);
        else if constexpr (ps<T>) return _mm256_mask_cmp_ps_mask(mask, as<__m256>(x), as<__m256>(y), _CMP_UNORD_Q);
    }
    else if constexpr (sizeof(V) == 64 && has_avx512f<ISA>) {
        if constexpr (pd<T>) return _mm512_mask_cmp_pd_mask(mask, as<__m512d>(x), as<__m512d>(y), _CMP_UNORD_Q);
        else if constexpr (ps<T>) return _mm512_mask_cmp_ps_mask(mask, as<__m512>(x), as<__m512>(y), _CMP_UNORD_Q);
    }

    return mask_and_<ISA, T>(is_unordered_<ISA, T>(x, y), mask);
}

__RAZE_VX_NAMESPACE_END

