#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Not {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x) const noexcept {
		return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, 0x55);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
		return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, 0x55, __mask);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
		return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, 0x55, __mask, __src);
	}
};

__RAZE_VX_NAMESPACE_END
