#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToGPR.h>
#include <src/raze/math/CountTrailingZeros.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_, bool _Unsafe_>
struct _Find_next_set {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline i32 operator()(_Tp_ __x, i32 __from) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) return __from == -1 ? !__x : 1;
		else {
			auto __mask = _To_mask<_ISA_, _Type_>()(__x);

			using _Mask_ = decltype(__mask);
			_Mask_ __clear_first_n_mask;
			
			__clear_first_n_mask = __from == -1 ? _Mask_(~_Mask_(0)) : _Mask_(_Mask_(~0) << (__from + 1));
			return math::__ctz_n_bits<_ISA_, _Size_, _Unsafe_>(_Mask_(__mask & __clear_first_n_mask));
		}
	}
};

__RAZE_VX_NAMESPACE_END
