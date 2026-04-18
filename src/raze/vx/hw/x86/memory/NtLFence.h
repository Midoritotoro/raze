#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Nt_lfence {
	raze_static_operator raze_always_inline void operator()() raze_const_operator noexcept {
#if defined(raze_processor_x86_64)
		_mm_lfence();
#endif // defined(raze_processor_x86)
	}
};

__RAZE_VX_NAMESPACE_END
