#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Nearest.h>
#include <src/raze/vx/hw/x86/compare/Less.h>
#include <cmath>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V ceil_(V x) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (has_sse41<ISA>) {
            if constexpr (pd<T>) return as<V>(_mm_ceil_pd(as<__m128d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm_ceil_ps(as<__m128>(x)));
        }
        else {
            const auto n = nearest_<ISA, T>(x);
            const auto lt = as<V>(less_<ISA, T>(n, x));
            const auto res = add_<ISA, T>(n, bit_and_<ISA, T>(lt, broadcast_<ISA, V>(T(1))));
            const auto sign = bit_and_<ISA, T>(x, broadcast_<ISA, V>(T(-0.0)));
            return bit_or_<ISA, T>(res, sign);
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (pd<T>) return as<V>(_mm256_ceil_pd(as<__m256d>(x)));
        else if constexpr (ps<T>) return as<V>(_mm256_ceil_ps(as<__m256>(x)));
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return as<V>(_mm512_roundscale_pd(as<__m512d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_roundscale_ps(as<__m512>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }
    else {
        return std::ceil(x);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V ceil_(V x, M mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_maskz_roundscale_pd(mask, as<__m128d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm_maskz_roundscale_ps(mask, as<__m128>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_maskz_roundscale_pd(mask, as<__m256d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm256_maskz_roundscale_ps(mask, as<__m256>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_maskz_roundscale_pd(mask, as<__m512d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_maskz_roundscale_ps(mask, as<__m512>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }

    return select_<ISA, T>(ceil_<ISA, T>(x), mask);
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V ceil_(V x, M mask, V src) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_mask_roundscale_pd(as<__m128d>(src), mask, as<__m128d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm_mask_roundscale_ps(as<__m128>(src), mask, as<__m128>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_mask_roundscale_pd(as<__m256d>(src), mask, as<__m256d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm256_mask_roundscale_ps(as<__m256>(src), mask, as<__m256>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_mask_roundscale_pd(as<__m512d>(src), mask, as<__m512d>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_mask_roundscale_ps(as<__m512>(src), mask, as<__m512>(x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
    }

    return select_<ISA, T>(ceil_<ISA, T>(x), src, mask);
}

__RAZE_VX_NAMESPACE_END

