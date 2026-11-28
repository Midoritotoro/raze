#pragma once

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <raze/math/BitCast.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto is_nan_(V x) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm_fpclass_pd_mask(as<__m128d>(x), 0x81);
            else if constexpr (ps<T>) return _mm_fpclass_ps_mask(as<__m128>(x), 0x81);
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (pd<T>) return _mm_cmp_pd_mask(as<__m128d>(x), as<__m128d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm_cmp_ps_mask(as<__m128>(x), as<__m128>(x), _CMP_UNORD_Q);
        }
        else {
            if constexpr (pd<T>) return _mm_cmpunord_pd(as<__m128d>(x), as<__m128d>(x));
            else if constexpr (ps<T>) return _mm_cmpunord_ps(as<__m128>(x), as<__m128>(x));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm256_fpclass_pd_mask(as<__m256d>(x), 0x81);
            else if constexpr (ps<T>) return _mm256_fpclass_ps_mask(as<__m256>(x), 0x81);
        }
        else if constexpr (has_avx512vl<ISA>) {
            if constexpr (pd<T>) return _mm256_cmp_pd_mask(as<__m256d>(x), as<__m256d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm256_cmp_ps_mask(as<__m256>(x), as<__m256>(x), _CMP_UNORD_Q);
        }
        else {
            if constexpr (pd<T>) return _mm256_cmp_pd(as<__m256d>(x), as<__m256d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm256_cmp_ps(as<__m256>(x), as<__m256>(x), _CMP_UNORD_Q);
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm512_fpclass_pd_mask(as<__m512d>(x), 0x81);
            else if constexpr (ps<T>) return _mm512_fpclass_ps_mask(as<__m512>(x), 0x81);
        }
        else {
            if constexpr (pd<T>) return _mm512_cmp_pd_mask(as<__m512d>(x), as<__m512d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm512_cmp_ps_mask(as<__m512>(x), as<__m512>(x), _CMP_UNORD_Q);
        }
    }
    else {
        if constexpr (pd<T>) return (math::bit_cast<u64>(x) & 0x7FFFFFFFFFFFFFFFull) > 0x7FF0000000000000ull;
        else return (math::bit_cast<u32>(x) & 0x7FFFFFFFu) > 0x7F800000u;
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto is_nan_(V x, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
        if constexpr (has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm_mask_fpclass_pd_mask(mask, as<__m128d>(x), 0x81);
            else if constexpr (ps<T>) return _mm_mask_fpclass_ps_mask(mask, as<__m128>(x), 0x81);
        }
        else {
            if constexpr (pd<T>) return _mm_mask_cmp_pd_mask(mask, as<__m128d>(x), as<__m128d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm_mask_cmp_ps_mask(mask, as<__m128>(x), as<__m128>(x), _CMP_UNORD_Q);
        }
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
        if constexpr (has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm256_mask_fpclass_pd_mask(mask, as<__m256d>(x), 0x81);
            else if constexpr (ps<T>) return _mm256_mask_fpclass_ps_mask(mask, as<__m256>(x), 0x81);
        }
        else {
            if constexpr (pd<T>) return _mm256_mask_cmp_pd_mask(mask, as<__m256d>(x), as<__m256d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm256_mask_cmp_ps_mask(mask, as<__m256>(x), as<__m256>(x), _CMP_UNORD_Q);
        }
    }
    else if constexpr (sizeof(V) == 64 && has_avx512f<ISA>) {
        if constexpr (has_avx512dq<ISA>) {
            if constexpr (pd<T>) return _mm512_mask_fpclass_pd_mask(mask, as<__m512d>(x), 0x81);
            else if constexpr (ps<T>) return _mm512_mask_fpclass_ps_mask(mask, as<__m512>(x), 0x81);
        }
        else {
            if constexpr (pd<T>) return _mm512_mask_cmp_pd_mask(mask, as<__m512d>(x), as<__m512d>(x), _CMP_UNORD_Q);
            else if constexpr (ps<T>) return _mm512_mask_cmp_ps_mask(mask, as<__m512>(x), as<__m512>(x), _CMP_UNORD_Q);
        }
    }

    return mask_and_<ISA, T>(is_nan_<ISA, T>(x), mask);
}

__RAZE_VX_NAMESPACE_END

