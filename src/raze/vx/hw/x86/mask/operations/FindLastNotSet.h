#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToGPR.h>
#include <src/raze/math/CountLeadingZeros.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_, bool _Unsafe_>
struct _Find_last_not_set {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline i32 operator()(_Tp_ __x) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return __x;
		}
		else {
			return math::__clz_not_n_bits<_ISA_, _Size_, _Unsafe_>(_To_mask<_ISA_, _Type_>()(__x));
		}
	}
};

__RAZE_VX_NAMESPACE_END
