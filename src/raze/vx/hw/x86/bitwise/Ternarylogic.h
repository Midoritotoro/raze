#pragma once 

#include <src/raze/vx/hw/x86/bitwise/emulated/Ternarylogic.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, u8 Op>
raze_always_inline V ternarylogic_(V x, V y, V z, std::integral_constant<u8, Op> op) noexcept {
    if constexpr (has_avx512f<ISA> && intrin_type<V>) {
        if constexpr (sizeof(V) == 64)  return as<V>(_mm512_ternarylogic_epi32(as<__m512i>(x), as<__m512i>(y), as<__m512i>(z), op));
        else if constexpr (has_avx512vl<ISA> && sizeof(V) == 32) return as<V>(_mm256_ternarylogic_epi32(as<__m256i>(x), as<__m256i>(y), as<__m256i>(z), op));
        else if constexpr (has_avx512vl<ISA> && sizeof(V) == 16) return as<V>(_mm_ternarylogic_epi32(as<__m128i>(x), as<__m128i>(y),  as<__m128i>(z), op));
    }

    return ternarylogic_emulated_(x, y, z, op,
        [] (auto a, auto b) raze_always_inline_lambda { return bit_or_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return bit_xor_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return bit_and_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return bit_andnot_<ISA, T>(a, b); },
        [] (auto a) raze_always_inline_lambda { return bit_not_<ISA, T>(a); },
        [] () raze_always_inline_lambda { return zero_<ISA, V>(); });
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, u8 Op, raw_mask_type M>
raze_always_inline V ternarylogic_(V x, V y,  V z, 
    std::integral_constant<u8, Op> op,  M mask) noexcept
{
    if constexpr (has_avx512f<ISA> && std::is_integral_v<M> && intrin_type<V>) {
        if constexpr (sizeof(V) == 64) {
            if constexpr (sizeof(T) == 8) return as<V>(_mm512_maskz_ternarylogic_epi64(mask, as<__m512i>(x), as<__m512i>(y), as<__m512i>(z), op));
            else if constexpr (sizeof(T) == 4) return as<V>(_mm512_maskz_ternarylogic_epi32(mask, as<__m512i>(x), as<__m512i>(y), as<__m512i>(z), op));
        }
        else if constexpr (has_avx512vl<ISA> && sizeof(V) == 32) {
            if constexpr (sizeof(T) == 8) return as<V>(_mm256_maskz_ternarylogic_epi64(mask, as<__m256i>(x),  as<__m256i>(y), as<__m256i>(z), op));
            else if constexpr (sizeof(T) == 4) return as<V>(_mm256_maskz_ternarylogic_epi32(mask, as<__m256i>(x), as<__m256i>(y), as<__m256i>(z), op));
        }
        else if constexpr (has_avx512vl<ISA> && sizeof(V) == 16) {
            if constexpr (sizeof(T) == 8) return as<V>(_mm_maskz_ternarylogic_epi64(mask, as<__m128i>(x), as<__m128i>(y), as<__m128i>(z), op));
            else if constexpr (sizeof(T) == 4) return as<V>(_mm_maskz_ternarylogic_epi32(mask, as<__m128i>(x), as<__m128i>(y), as<__m128i>(z), op));
        }
    }
    
    return select_<ISA, T>(ternarylogic_<ISA, T>(x, y, z, op), mask);
}


template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, u8 Op, raw_mask_type M>
raze_always_inline V ternarylogic_(V x, V y,  V z, 
    std::integral_constant<u8, Op> op, M mask, V src)  noexcept
{
    return select_<ISA, T>(ternarylogic_<ISA, T>(x, y, z, op), src, mask);
}

__RAZE_VX_NAMESPACE_END
