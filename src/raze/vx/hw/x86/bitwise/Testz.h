#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/ToBitmask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Testz {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline bool 
		operator()(_IntrinType_ __vector) const noexcept
	{
		/*const auto __zeros = _Zero<arch::ISA::SSE2, 128, _IntrinType_>()();
		const auto __compared = _Equal<arch::ISA::SSE2, 128, int32>()(__vector, __zeros);

		const auto __index_mask = _To_bitmask<arch::ISA::SSE2, 128, int32>()(__compared);
		return __index_mask == 0xF;*/
	}
};


__RAZE_VX_NAMESPACE_END
