#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, intrin_or_arithmetic_type _Tp_>
struct _All_ones {
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()() raze_const_operator noexcept {
		return _Broadcast<_ISA_, _Tp_>()(-1);
	}
};

__RAZE_VX_NAMESPACE_END
