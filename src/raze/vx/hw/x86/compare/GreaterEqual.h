#pragma once 

#include <src/raze/vx/hw/x86/compare/LessEqual.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	    _ISA_,
	arithmetic_type	_Type_>
struct _Greater_equal {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		return _Less_equal<_ISA_, _Type_>()(__y, __x);
	}
};

__RAZE_VX_NAMESPACE_END
