#pragma once 

#include <src/raze/vx/arithmetic/Sub.h>
#include <src/raze/vx/shuffle/BroadcastZeros.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA	_ISA_>
struct _Nt_sfence {
	raze_static_operator raze_always_inline void operator()() raze_const_operator noexcept {
#if defined(raze_processor_x86_64)
		_mm_sfence();
#endif // defined(raze_processor_x86)
	}
};

__RAZE_VX_NAMESPACE_END
