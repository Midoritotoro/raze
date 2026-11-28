#pragma once

#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/compare/NotEqual.h>
#include <src/raze/vx/hw/x86/compare/IsNotFinite.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <raze/math/BitCast.h>
#include <limits>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto is_finite_(V x) noexcept {
    if constexpr ((sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) ||
                  (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) ||
                  (sizeof(V) == 64 && has_avx512dq<ISA>))
    {
        return mask_not_<ISA, sizeof(V) / sizeof(T), T>(is_not_finite_<ISA, T>(x));
    }
    else if constexpr (arithmetic_type<V>) {
        if constexpr (pd<T>) return (math::bit_cast<u64>(x) & 0x7FF0000000000000ull) != 0x7FF0000000000000ull;
        else return (math::bit_cast<u32>(x) & 0x7F800000u) != 0x7F800000u;
    }
    else {
        const auto inf = broadcast_<ISA, V>(std::numeric_limits<T>::infinity());
        return not_equal_<ISA, T>(bit_and_<ISA, T>(x, inf), inf);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto is_finite_(V x, Mask mask) noexcept {
    if constexpr ((sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) ||
                  (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) ||
                  (sizeof(V) == 64 && has_avx512dq<ISA>))
    {
        return mask_and_<ISA, T>(mask_not_<ISA, sizeof(V) / sizeof(T), T>(is_not_finite_<ISA, T>(x, mask)), mask);
    }
    else if constexpr (!arithmetic_type<V>) {
        const auto inf = broadcast_<ISA, V>(std::numeric_limits<T>::infinity());
        return not_equal_<ISA, T>(bit_and_<ISA, T>(x, inf), inf, mask);
    }
    else {
        return mask_and_<ISA, T>(is_finite_<ISA, T>(x), mask);
    }
}

__RAZE_VX_NAMESPACE_END

