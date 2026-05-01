#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, intrin_type _Intrin_, arithmetic_type _Type_>
struct _Any_of {
	template <raw_mask_type _Tp_>
	raze_nodisard raze_always_inline bool operator()(_Tp_ __x) const noexcept {
		return !_None_of<_ISA_, _Intrin_, _Type_>()(__x);
	}
};

__RAZE_VX_NAMESPACE_END
