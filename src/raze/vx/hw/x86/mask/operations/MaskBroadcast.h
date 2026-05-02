#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, raw_mask_type _Mask_, arithmetic_type _Type_>
struct _Mask_broadcast {
	raze_nodiscard raze_static_operator raze_always_inline _Mask_ operator()(bool __v) raze_const_operator noexcept {
		if constexpr (intrin_type<_Mask_>)
			return __v ? _Broadcast<_ISA_, _Mask_>()(-1) : _Zero<_ISA_, _Mask_>()();
		else
			return _First_n<_ISA_, _Size_, _Mask_, _Type_>()(_Size_ * i32(__v));
	}
};

__RAZE_VX_NAMESPACE_END
