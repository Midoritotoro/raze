#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
#include <src/raze/vx/hw/x86/mask/operations/BitLeftShift.h>
#include <src/raze/vx/hw/x86/mask/operations/BitRightShift.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Is_contiguous {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __mask, i32 __n, i32 __k) const noexcept {
		if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
			return (__k <= __n) ? true : __mask != 0;
		}
		else if constexpr (std::is_integral_v<_Tp_>) {
			auto __len = __k - __n;
			
#if 0
			u32 __is_valid = static_cast<u32>(__len > 0);
			u32 __shift = (_Size_ - __len) & -__is_valid;
			auto __x = (__max_for_bits<_Size_, _Tp_>() >> __shift) & -__is_valid;
			__x <<= __n;
			return (__mask & __x) == __x;
#else
			if constexpr (_Size_ == 64) return _tzcnt_u64((~__mask) >> __n) >= __len;
			else return (((~__mask) >> __n) & ((1ull << __len) - 1)) == 0;
#endif
		}
		else {
			return _Is_contiguous()(_To_mask<_ISA_, _Type_>()(__mask), __n, __k);
		}
	}
};

__RAZE_VX_NAMESPACE_END
