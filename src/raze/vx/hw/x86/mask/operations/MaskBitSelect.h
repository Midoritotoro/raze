#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAndnot.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_bitselect_(M x, M src, M mask) noexcept {
	if constexpr (std::is_same_v<M, bool>) return mask ? x : src;
	else if constexpr (intrin_type<M>) return select_<ISA, T>(x, src, mask);
	else return mask_or_<ISA, T>(mask_and_<ISA, T>(x, mask), mask_andnot_<ISA, T>(src, mask));
}

__RAZE_VX_NAMESPACE_END
