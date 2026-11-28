#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Abs.h>
#include <src/raze/vx/hw/x86/arithmetic/Add.h>
#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/compare/Less.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <cmath>

__RAZE_VX_NAMESPACE_BEGIN

template <std::floating_point T>
raze_nodiscard consteval T two_to_mantissa_bits_() noexcept {
    if constexpr (pd<T>) return T(4503599627370496.0);
    else return T(8388608.0f);
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V nearest_(V x) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (has_sse41<ISA>) {
            if constexpr (pd<T>) return as<V>(_mm_round_pd(as<__m128d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
            else if constexpr (ps<T>) return as<V>(_mm_round_ps(as<__m128>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        }
        else {
            const auto two_n = broadcast_<ISA, V>(two_to_mantissa_bits_<T>());
            const auto sign = bit_and_<ISA, T>(x, broadcast_<ISA, V>(T(-0.0)));
            const auto s = bit_or_<ISA, T>(sign, two_n);
            const auto r = bit_or_<ISA, T>(sub_<ISA, T>(add_<ISA, T>(x, s), s), sign);
            const auto in_range = less_<ISA, T>(abs_<ISA, T>(x), two_n);
            return select_<ISA, T>(r, x, in_range);
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (pd<T>) return as<V>(_mm256_round_pd(as<__m256d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm256_round_ps(as<__m256>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (pd<T>) return as<V>(_mm512_roundscale_pd(as<__m512d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_roundscale_ps(as<__m512>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else {
        return std::nearbyint(x);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V nearest_(V x, M mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_maskz_roundscale_pd(mask, as<__m128d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm_maskz_roundscale_ps(mask, as<__m128>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_maskz_roundscale_pd(mask, as<__m256d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm256_maskz_roundscale_ps(mask, as<__m256>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_maskz_roundscale_pd(mask, as<__m512d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_maskz_roundscale_ps(mask, as<__m512>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }

    return select_<ISA, T>(nearest_<ISA, T>(x), mask);
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V nearest_(V x, M mask, V src) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm_mask_roundscale_pd(as<__m128d>(src), mask, as<__m128d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm_mask_roundscale_ps(as<__m128>(src), mask, as<__m128>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm256_mask_roundscale_pd(as<__m256d>(src), mask, as<__m256d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm256_mask_roundscale_ps(as<__m256>(src), mask, as<__m256>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        if constexpr (pd<T>) return as<V>(_mm512_mask_roundscale_pd(as<__m512d>(src), mask, as<__m512d>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
        else if constexpr (ps<T>) return as<V>(_mm512_mask_roundscale_ps(as<__m512>(src), mask, as<__m512>(x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    }

    return select_<ISA, T>(nearest_<ISA, T>(x), src, mask);
}

__RAZE_VX_NAMESPACE_END
