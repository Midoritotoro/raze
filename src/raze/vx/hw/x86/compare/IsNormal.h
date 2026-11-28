#pragma once

#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/compare/NotEqual.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <raze/math/BitCast.h>
#include <limits>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto is_normal_(V x) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm_fpclass_pd_mask(as<__m128d>(x), 0xBF));
        else if constexpr (ps<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm_fpclass_ps_mask(as<__m128>(x), 0xBF));
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm256_fpclass_pd_mask(as<__m256d>(x), 0xBF));
        else if constexpr (ps<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm256_fpclass_ps_mask(as<__m256>(x), 0xBF));
    }
    else if constexpr (sizeof(V) == 64 && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm512_fpclass_pd_mask(as<__m512d>(x), 0xBF));
        else if constexpr (ps<T>) return mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm512_fpclass_ps_mask(as<__m512>(x), 0xBF));
    }
    else if constexpr (arithmetic_type<V>) {
        if constexpr (pd<T>) {
            const u64 e = math::bit_cast<u64>(x) & 0x7FF0000000000000ull;
            return e != 0ull && e != 0x7FF0000000000000ull;
        }
        else {
            const u32 e = math::bit_cast<u32>(x) & 0x7F800000u;
            return e != 0u && e != 0x7F800000u;
        }
    }
    else {
        const auto inf = broadcast_<ISA, V>(std::numeric_limits<T>::infinity());
        const auto exp = bit_and_<ISA, T>(x, inf);
        return mask_and_<ISA, T>(not_equal_<ISA, T>(exp, zero_<ISA, V, T>()), not_equal_<ISA, T>(exp, inf));
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto is_normal_(V x, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm_mask_fpclass_pd_mask(mask, as<__m128d>(x), 0xBF)), mask);
        else if constexpr (ps<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm_mask_fpclass_ps_mask(mask, as<__m128>(x), 0xBF)), mask);
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm256_mask_fpclass_pd_mask(mask, as<__m256d>(x), 0xBF)), mask);
        else if constexpr (ps<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm256_mask_fpclass_ps_mask(mask, as<__m256>(x), 0xBF)), mask);
    }
    else if constexpr (sizeof(V) == 64 && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm512_mask_fpclass_pd_mask(mask, as<__m512d>(x), 0xBF)), mask);
        else if constexpr (ps<T>) return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(_mm512_mask_fpclass_ps_mask(mask, as<__m512>(x), 0xBF)), mask);
    }

    return mask_and_<ISA, T>(is_normal_<ISA, T>(x), mask);
}

__RAZE_VX_NAMESPACE_END

