#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <arch::ISA	_ISA_>
struct _Simd_streaming_fence {
	raze_static_operator raze_always_inline void operator()() raze_const_operator noexcept {
#if defined(raze_processor_x86_64)
		_mm_sfence();
#endif // defined(raze_processor_x86)
	}
};

__RAZE_DATAPAR_NAMESPACE_END
