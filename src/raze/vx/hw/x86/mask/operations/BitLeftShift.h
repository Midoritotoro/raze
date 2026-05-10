#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Mask_lshift {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, i32 __shift) raze_const_operator noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return __x;
		}
		else if constexpr (std::is_integral_v<_Tp_>) {
			raze_maybe_unused_attribute constexpr auto __all_mask = ((sizeof(_Tp_) * 8) == _Size_)
				? math::__maximum_integral_limit<_Tp_>() : _Tp_(((_Tp_(1) << _Size_) - 1));

			if constexpr (_Size_ < 8) return (__x << __shift) & __all_mask;
			else return (__x << __shift);
		}
		else {
			
		}
	}
};

__RAZE_VX_NAMESPACE_END
