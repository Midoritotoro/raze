#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Trunc.h>
#include <src/raze/vx/hw/x86/arithmetic/CopySign.h>
#include <raze/math/BitCast.h>
#include <cmath>

__RAZE_VX_NAMESPACE_BEGIN

template <std::floating_point T>
raze_nodiscard consteval T prev_half_constant_() noexcept {
    if constexpr (pd<T>) return math::bit_cast<T>(u64(0x3FDFFFFFFFFFFFFFull));
    else return math::bit_cast<T>(u32(0x3EFFFFFFu));
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V round_(V x) noexcept {
    if constexpr (arithmetic_type<V>) {
        return std::round(x);
    }
    else {
        const auto prev_half = broadcast_<ISA, V>(prev_half_constant_<T>());
        return trunc_<ISA, T>(add_<ISA, T>(x, copysign_<ISA, T>(prev_half, x)));
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V round_(V x, M mask) noexcept {
    if constexpr (arithmetic_type<V>) {
        return mask ? std::round(x) : V(0);
    }
    else {
        const auto prev_half = broadcast_<ISA, V>(prev_half_constant_<T>());
        return trunc_<ISA, T>(add_<ISA, T>(x, copysign_<ISA, T>(prev_half, x)), mask);
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V round_(V x, M mask, V src) noexcept {
    if constexpr (arithmetic_type<V>) {
        return mask ? std::round(x) : src;
    }
    else {
        const auto prev_half = broadcast_<ISA, V>(prev_half_constant_<T>());
        return trunc_<ISA, T>(add_<ISA, T>(x, copysign_<ISA, T>(prev_half, x)), mask, src);
    }
}

__RAZE_VX_NAMESPACE_END
