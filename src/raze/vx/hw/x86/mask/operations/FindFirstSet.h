#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToGPR.h>
#include <src/raze/math/CountTrailingZeros.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Find_first_set {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline i32 operator()(_Tp_ __x) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return !__x;
		}
		else {
			if constexpr (_Size_ <= 8) return math::__ctz_n_bits<_Size_>(__to_gpr<_ISA_>(_To_mask<_ISA_, _Type_>()(__x)));
			else if constexpr (__has_avx2_support_v<_ISA_>) return math::__tzcnt_ctz(__to_gpr<_ISA_>(_To_mask<_ISA_, _Type_>()(__x)));
			else return math::__bsf_ctz(__to_gpr<_ISA_>(_To_mask<_ISA_, _Type_>()(__x)));
		}
	}
};

__RAZE_VX_NAMESPACE_END
