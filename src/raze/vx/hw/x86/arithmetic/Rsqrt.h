#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Sqrt.h>
#include <src/raze/vx/hw/x86/arithmetic/Div.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, bool Raw = false, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V rsqrt_(V x) noexcept {
    if constexpr (Raw) {
        if constexpr (sizeof(V) == 16) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (pd<T>) return as<V>(_mm_rsqrt14_pd(as<__m128d>(x)));
                else if constexpr (ps<T>) return as<V>(_mm_rsqrt14_ps(as<__m128>(x)));
            }
            else if constexpr (ps<T>) {
                return as<V>(_mm_rsqrt_ps(as<__m128>(x)));
            }
        }
        else if constexpr (sizeof(V) == 32) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (pd<T>) return as<V>(_mm256_rsqrt14_pd(as<__m256d>(x)));
                else if constexpr (ps<T>) return as<V>(_mm256_rsqrt14_ps(as<__m256>(x)));
            }
            else if constexpr (ps<T>) {
                return as<V>(_mm256_rsqrt_ps(as<__m256>(x)));
            }
        }
        else if constexpr (sizeof(V) == 64) {
            if constexpr (pd<T>) return as<V>(_mm512_rsqrt14_pd(as<__m512d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm512_rsqrt14_ps(as<__m512>(x)));
        }
    }

    if constexpr (arithmetic_type<V>)
        return static_cast<V>(T(1) / std::sqrt(x));
    else
        return div_<ISA, T>(broadcast_<ISA, V>(T(1)), sqrt_<ISA, T>(x));
}

template <arch::ISA ISA, std::floating_point T, bool Raw = false, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rsqrt_(V x, M mask) noexcept {
    if constexpr (Raw) {
        if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm_maskz_rsqrt14_pd(mask, as<__m128d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm_maskz_rsqrt14_ps(mask, as<__m128>(x)));
        }
        else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm256_maskz_rsqrt14_pd(mask, as<__m256d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm256_maskz_rsqrt14_ps(mask, as<__m256>(x)));
        }
        else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm512_maskz_rsqrt14_pd(mask, as<__m512d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm512_maskz_rsqrt14_ps(mask, as<__m512>(x)));
        }

        return select_<ISA, T>(rsqrt_<ISA, T, true>(x), mask);
    }
    else {
        if constexpr (arithmetic_type<V>)
            return mask ? static_cast<V>(T(1) / std::sqrt(x)) : V(0);
        else
            return div_<ISA, T>(broadcast_<ISA, V>(T(1)), sqrt_<ISA, T>(x), mask);
    }
}

template <arch::ISA ISA, std::floating_point T, bool Raw = false, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rsqrt_(V x, M mask, V src) noexcept {
    if constexpr (Raw) {
        if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm_mask_rsqrt14_pd(as<__m128d>(src), mask, as<__m128d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm_mask_rsqrt14_ps(as<__m128>(src), mask, as<__m128>(x)));
        }
        else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm256_mask_rsqrt14_pd(as<__m256d>(src), mask, as<__m256d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm256_mask_rsqrt14_ps(as<__m256>(src), mask, as<__m256>(x)));
        }
        else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
            if constexpr (pd<T>) return as<V>(_mm512_mask_rsqrt14_pd(as<__m512d>(src), mask, as<__m512d>(x)));
            else if constexpr (ps<T>) return as<V>(_mm512_mask_rsqrt14_ps(as<__m512>(src), mask, as<__m512>(x)));
        }

        return select_<ISA, T>(rsqrt_<ISA, T, true>(x), src, mask);
    }
    else {
        if constexpr (arithmetic_type<V>)
            return mask ? static_cast<V>(T(1) / std::sqrt(x)) : src;
        else
            return div_<ISA, T>(broadcast_<ISA, V>(T(1)), sqrt_<ISA, T>(x), mask, src);
    }
}

__RAZE_VX_NAMESPACE_END

