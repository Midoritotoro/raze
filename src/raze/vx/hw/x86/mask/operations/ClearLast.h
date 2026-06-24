#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/math/SignBit.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, sizetype _Size_, bool _Unsafe_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline _Tp_ __clear_first(_Tp_ __x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
		return 0;
	}
	else if constexpr (std::is_integral_v<_Tp_>) {
		if constexpr (__has_avx2_support_v<_ISA_>) {
			if constexpr (_Size_ == 64) return _blsr_u64(__x);
			else return _blsr_u32(__x);
		}
		else return __x & (__x - 1);
	}
	else if constexpr (intrin_type<_Tp_>) {
		return _To_vector<_ISA_, _Tp_, _Type_>()(__clear_first<_ISA_, _Type_, _Size_, _Unsafe_>(_To_mask<_ISA_, _Type_>()(__x)));
	}
}

__RAZE_VX_NAMESPACE_END
