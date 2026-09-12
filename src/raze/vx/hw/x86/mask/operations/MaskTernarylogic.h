#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskXor.h>
#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelect.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelectz.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, sizetype N, arithmetic_type T, raw_mask_type M, u8 Op>
raze_always_inline M mask_ternarylogic_(M x, M y, M z, std::integral_constant<u8, Op> op) noexcept {
    if constexpr (intrin_type<M>) return ternarylogic_<ISA, T>(x, y, z, op);
    else return ternarylogic_emulated_(x, y, z, op, 
        [] (auto a, auto b) raze_always_inline_lambda { return mask_or_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return mask_xor_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return mask_and_<ISA, T>(a, b); },
        [] (auto a, auto b) raze_always_inline_lambda { return mask_andnot_<ISA, T>(a, b); },
        [] (auto a) raze_always_inline_lambda { return mask_not_<ISA, N, T>(a); },
        []() raze_always_inline_lambda { return 0; });
}

template <arch::ISA ISA, sizetype N, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask, u8 Op>
raze_always_inline M mask_ternarylogic_(M x, M y, M z, std::integral_constant<u8, Op> op, ControlMask mask) noexcept {
    return mask_bitselectz_<ISA, T>(mask_ternarylogic_<ISA, N, T>(x, y, z, op), mask);
}

template <arch::ISA ISA, sizetype N, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask, u8 Op>
raze_always_inline M mask_ternarylogic_(M x, M y, M z, std::integral_constant<u8, Op> op, ControlMask mask, M src) noexcept {
    return mask_bitselect_<ISA, T>(mask_ternarylogic_<ISA, N, T>(x, y, z, op), src, mask);
}

__RAZE_VX_NAMESPACE_END
