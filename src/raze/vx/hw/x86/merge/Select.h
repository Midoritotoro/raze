#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Select {
	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		return _Ternarylogic<_ISA_, _Type_>()(__mask, __x, __y, 0xac);
	}
};


__RAZE_VX_NAMESPACE_END
