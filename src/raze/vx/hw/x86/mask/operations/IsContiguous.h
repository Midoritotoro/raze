#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
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
			constexpr auto __first_n_table = __first_n_ktable<_Size_>();

			const auto __size = __k - __n + 1;
			auto __x = _Tp_(0);

			if (__size >= sizeof(_Tp_) * 8) __x = math::__maximum_integral_limit<_Tp_>();
			else __x = _Mask_lshift<_ISA_, _Size_, _Type_>()(__first_n_table[__size], __n);

			return _Mask_and<_ISA_, _Type_>()(__mask, __x) == __x;
		}
		else {
			return _Is_contiguous()(_To_mask<_ISA_, _Type_>()(__mask), __n, __k);
		}
	}
};

__RAZE_VX_NAMESPACE_END
