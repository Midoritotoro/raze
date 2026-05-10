#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Mask_rshift {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, i32 __shift) raze_const_operator noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return !__shift;
		}
		else if constexpr (std::is_integral_v<_Tp_>) {
			return (__shift >= _Size_) ? 0 : __x >> __shift;
		}
		else {
			
		}
	}
};

__RAZE_VX_NAMESPACE_END
