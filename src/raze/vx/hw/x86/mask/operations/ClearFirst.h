#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAndnot.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskXor.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/math/SignBit.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, sizetype _Size_, bool _Unsafe_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline _Tp_ __clear_last(_Tp_ __x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
		return 0;
	}
	else if constexpr (std::is_integral_v<_Tp_>) {
		if constexpr (_Size_ >= 32) {
			return _Mask_xor<_ISA_, _Type_>()(__x, (math::sign_bit<_Tp_>() >>
				math::__clz_n_bits<_ISA_, _Size_, _Unsafe_>(__x)));
		}
		else {
			const auto __pos = (_Size_ - 1) - math::__clz_n_bits<_ISA_, _Size_, _Unsafe_>(__x);
			return _Mask_andnot<_ISA_, _Type_>()(_Tp_(_Tp_(1) << __pos), __x);
		}
	}
	else if constexpr (intrin_type<_Tp_>) {
		return _To_vector<_ISA_, _Tp_, _Type_>()(__clear_last<_ISA_, _Type_, _Size_, _Unsafe_>(_To_mask<_ISA_, _Type_>()(__x)));
	}
}

__RAZE_VX_NAMESPACE_END
