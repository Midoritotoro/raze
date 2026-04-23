#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	        _ISA_,
    arithmetic_type     _Element_>
struct _Extract {
    template <intrin_or_arithmetic_type _Tp_>
    raze_nodiscard raze_static_operator raze_always_inline _Element_ operator()(
        _Tp_    __vector,
        uint8	__index) raze_const_operator noexcept
    {
        if constexpr (arithmetic_type<_Tp_>) {
            return __vector;
        }
        else {
            constexpr auto __length = sizeof(_Tp_) / sizeof(_Element_);

            alignas(sizeof(_Tp_)) _Element_ __array[__length];
            _Store<_ISA_>()(__array, __vector);

            return __array[__index & (__length - 1)];
        }
    }
};

__RAZE_VX_NAMESPACE_END
