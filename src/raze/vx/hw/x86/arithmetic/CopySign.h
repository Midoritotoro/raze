#pragma once

#include <src/raze/vx/hw/x86/bitwise/Andnot.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <cmath>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V copysign_(V x, V y) noexcept {
    if constexpr (arithmetic_type<V>) {
        return std::copysign(x, y);
    }
    else {
        const auto sign_mask = broadcast_<ISA, V>(T(-0.0));

        if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>)
            return as<V>(_mm_ternarylogic_epi32(as<__m128i>(sign_mask), as<__m128i>(y), as<__m128i>(x), 0xCA));
        else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>)
            return as<V>(_mm256_ternarylogic_epi32(as<__m256i>(sign_mask), as<__m256i>(y), as<__m256i>(x), 0xCA));
        else if constexpr (sizeof(V) == 64)
            return as<V>(_mm512_ternarylogic_epi32(as<__m512i>(sign_mask), as<__m512i>(y), as<__m512i>(x), 0xCA));
        else
            return bit_or_<ISA, T>(bit_andnot_<ISA, T>(sign_mask, x), bit_and_<ISA, T>(sign_mask, y));
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V copysign_(V x, V y, M mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        const auto sign_mask = broadcast_<ISA, V>(T(-0.0));
        if constexpr (pd<T>) return as<V>(_mm_maskz_ternarylogic_epi64(mask, as<__m128i>(sign_mask), as<__m128i>(y), as<__m128i>(x), 0xCA));
        else if constexpr (ps<T>) return as<V>(_mm_maskz_ternarylogic_epi32(mask, as<__m128i>(sign_mask), as<__m128i>(y), as<__m128i>(x), 0xCA));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
        const auto sign_mask = broadcast_<ISA, V>(T(-0.0));
        if constexpr (pd<T>) return as<V>(_mm256_maskz_ternarylogic_epi64(mask, as<__m256i>(sign_mask), as<__m256i>(y), as<__m256i>(x), 0xCA));
        else if constexpr (ps<T>) return as<V>(_mm256_maskz_ternarylogic_epi32(mask, as<__m256i>(sign_mask), as<__m256i>(y), as<__m256i>(x), 0xCA));
    }
    else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
        const auto sign_mask = broadcast_<ISA, V>(T(-0.0));
        if constexpr (pd<T>) return as<V>(_mm512_maskz_ternarylogic_epi64(mask, as<__m512i>(sign_mask), as<__m512i>(y), as<__m512i>(x), 0xCA));
        else if constexpr (ps<T>) return as<V>(_mm512_maskz_ternarylogic_epi32(mask, as<__m512i>(sign_mask), as<__m512i>(y), as<__m512i>(x), 0xCA));
    }

    return select_<ISA, T>(copysign_<ISA, T>(x, y), mask);
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V copysign_(V x, V y, M mask, V src) noexcept {
    return select_<ISA, T>(copysign_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END

