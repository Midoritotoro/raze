#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_rshift_(M x, i32 shift) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return !shift;
	}
	else if constexpr (std::is_integral_v<M>) {
		return (shift >= N) ? 0 : x >> shift;
	}
	else {
		
	}
}

__RAZE_VX_NAMESPACE_END
