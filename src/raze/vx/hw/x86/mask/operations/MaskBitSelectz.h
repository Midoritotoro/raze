#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelect.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Mask_bit_selectz {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __mask) raze_const_operator noexcept {
		if constexpr (intrin_type<_Tp_>)
			return _Select<_ISA_, _Type_>()(__x, __mask);
		else
			return _Mask_and<_ISA_, _Type_>()(__x, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
