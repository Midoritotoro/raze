#pragma once 

#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Selectz  {
	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_	__mask) const noexcept {
		if constexpr (sizeof(_Tp_) == )
		return _Select<_ISA_, _Type_>()(__x, _Zero<_ISA_, _Tp_>()(), __mask);
	}
};

__RAZE_VX_NAMESPACE_END
