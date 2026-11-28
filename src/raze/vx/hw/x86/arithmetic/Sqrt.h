#pragma once

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <cmath>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V sqrt_(V x) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (pd<T>) return as<V>(_mm_sqrt_pd(as<__m128d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm_sqrt_ps(as<__m128>(x)));
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (pd<T>) return as<V>(_mm256_sqrt_pd(as<__m256d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm256_sqrt_ps(as<__m256>(x)));
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return as<V>(_mm512_sqrt_pd(as<__m512d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm512_sqrt_ps(as<__m512>(x)));
    }
    else {
        return std::sqrt(x);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V sqrt_(V x, M mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_maskz_sqrt_pd(mask, as<__m128d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm_maskz_sqrt_ps(mask, as<__m128>(x)));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_maskz_sqrt_pd(mask, as<__m256d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm256_maskz_sqrt_ps(mask, as<__m256>(x)));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_maskz_sqrt_pd(mask, as<__m512d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm512_maskz_sqrt_ps(mask, as<__m512>(x)));
    }

    return select_<ISA, T>(sqrt_<ISA, T>(x), mask);
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V sqrt_(V x, M mask, V src) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_mask_sqrt_pd(as<__m128d>(src), mask, as<__m128d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm_mask_sqrt_ps(as<__m128>(src), mask, as<__m128>(x)));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_mask_sqrt_pd(as<__m256d>(src), mask, as<__m256d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm256_mask_sqrt_ps(as<__m256>(src), mask, as<__m256>(x)));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_mask_sqrt_pd(as<__m512d>(src), mask, as<__m512d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm512_mask_sqrt_ps(as<__m512>(src), mask, as<__m512>(x)));
    }

    return select_<ISA, T>(sqrt_<ISA, T>(x), src, mask);
}

__RAZE_VX_NAMESPACE_END

