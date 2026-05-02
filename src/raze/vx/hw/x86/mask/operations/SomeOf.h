#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/AllOf.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u64 _Size_, arithmetic_type _Type_>
struct _Some_of {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __x) const noexcept {
		return !_All_of<_ISA_, _Size_, _Type_>()(__x);
	}
};

__RAZE_VX_NAMESPACE_END
