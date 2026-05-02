#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAndnot.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Mask_bit_select {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __src, _Tp_ __mask) raze_const_operator noexcept {
		if constexpr (std::is_same_v<_Tp_, bool>)
			return __mask ? __x : __src;
		else if constexpr (intrin_type<_Tp_>)
			return _Select<_ISA_, _Type_>()(__x, __src, __mask);
		else
			return _Mask_or<_ISA_, _Type_>()(_Mask_and<_ISA_, _Type_>()(__x, __mask), _Mask_andnot<_ISA_, _Type_>()(__src, __mask));
	}
};

__RAZE_VX_NAMESPACE_END
