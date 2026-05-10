#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/BitLeftShift.h>



__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Is_contiguous {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __mask, i32 __n, i32 __k) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return __mask != 0;
		}
		else if constexpr (std::is_integral_v<_Tp_>) {
			const auto __size = __k - __n + 1;

			_Tp_ __x = _Tp_((math::__maximum_integral_limit<_Tp_>()) >> (_Size_ - __size));
			__x = _Mask_lshift<_ISA_, _Size_, _Type_>()(__x, __n);

			__x = _Mask_or<_ISA_, _Type_>()(__x, _Mask_and<_ISA_, _Type_>()(_Tp_(_Mask_not<_ISA_, _Type_>()(__x)), _Tp_(_Tp_(0) - (__size >= _Size_))));
			return _Mask_and<_ISA_, _Type_>()(__mask, __x) == __x;
		}
		else {
			return _Is_contiguous()(_To_mask<_ISA_, _Type_>()(__mask), __n, __k);
		}
	}
};

__RAZE_VX_NAMESPACE_END
