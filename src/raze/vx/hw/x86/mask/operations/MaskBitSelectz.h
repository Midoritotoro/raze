#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/MaskBitSelect.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_bitselectz_(M x, M mask) noexcept {
	if constexpr (intrin_type<M>) return select_<ISA, T>(x, mask);
	else return mask_and_<ISA, T>(x, mask);
}

__RAZE_VX_NAMESPACE_END
