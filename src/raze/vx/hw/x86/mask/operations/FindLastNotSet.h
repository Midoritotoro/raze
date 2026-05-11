#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToGPR.h>
#include <src/raze/math/CountLeadingZeros.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Find_last_not_set {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline i32 operator()(_Tp_ __x) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return __x;
		}
		else {
			auto __int_mask = _To_mask<_ISA_, _Type_>()(__x);
			using _MaskTp = decltype(__int_mask);

			if constexpr (_Size_ <= 8) return math::__clz_n_bits<_Size_>(__to_gpr<_ISA_>(_MaskTp(~__int_mask)));
			else if constexpr (__has_avx2_support_v<_ISA_>) return math::__lzcnt_clz(__to_gpr<_ISA_>(_MaskTp(~__int_mask)));
			else return math::__bsr_clz(__to_gpr<_ISA_>(_MaskTp(~__int_mask)));
		}
	}
};

__RAZE_VX_NAMESPACE_END
