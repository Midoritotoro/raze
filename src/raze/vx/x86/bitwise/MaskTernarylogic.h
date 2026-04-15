#pragma once 

#include <src/raze/vx/bitwise/Ternarylogic.h>
#include <src/raze/vx/shuffle/Blend.h>


__RAZE_VX_NAMESPACE_BEGIN


template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _Type_>
struct _Mask_ternarylogic {
    template <
        class _IntrinType_,
        class _MaskType_,  
        uint8 _TernaryMask_>
    raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
        _IntrinType_                                    __x,
        _IntrinType_                                    __y,
        _IntrinType_                                    __z,
        std::integral_constant<uint8, _TernaryMask_>    __imm8,
        _MaskType_                                      __mask,
        _IntrinType_                                    __source) raze_const_operator noexcept
    {
        return _Blend<_ISA_, _Width_, _Type_>()(
            _Ternarylogic<_ISA_, _Width_>()(__x, __y, __z, __imm8),
            __source, __mask);
    }
};

__RAZE_VX_NAMESPACE_END
