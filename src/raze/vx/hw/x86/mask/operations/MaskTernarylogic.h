#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskXor.h>
#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelect.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelectz.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Mask_ternarylogic {
    template <raw_mask_type _Tp_, u8 _Op_>
    raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(
        _Tp_ __x, _Tp_ __y,  _Tp_ __z, std::integral_constant<u8, _Op_> __imm8) raze_const_operator noexcept
    {
        if constexpr (intrin_type<_Tp_>)
            return _Ternarylogic<_ISA_, _Type_>()(__x, __y, __z, __imm8);
        else
            return _Ternarylogic_emulated()(__x, __y, __z, __imm8, _Mask_or<_ISA_, _Type_>(), _Mask_xor<_ISA_, _Type_>(),
                _Mask_and<_ISA_, _Type_>(), _Mask_andnot<_ISA_, _Type_>(), _Mask_not<_ISA_, _Type_>(), []() raze_always_inline_lambda { return 0; });
    }

    template <raw_mask_type _Tp_, raw_mask_type _Mask_, u8 _Op_>
    raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y,  _Tp_ __z, 
        std::integral_constant<u8, _Op_> __imm8,  _Mask_ __mask) const noexcept
    {
        return _Mask_bit_selectz<_ISA_, _Type_>()((*this)(__x, __y, __z, __imm8), __mask);
    }


    template <raw_mask_type _Tp_, raw_mask_type _Mask_, u8 _Op_>
    raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y,  _Tp_ __z, 
        std::integral_constant<u8, _Op_> __imm8,  _Mask_ __mask, _Tp_ __src) const noexcept
    {
        return _Mask_bit_select<_ISA_, _Type_>()((*this)(__x, __y, __z, __imm8), __src, __mask);
    }
};


__RAZE_VX_NAMESPACE_END
