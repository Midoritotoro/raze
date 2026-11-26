#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_rshift_(M x, i32 shift) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return (shift == 0) ? x : false;
	}
	else if constexpr (std::is_integral_v<M>) {
		return (shift >= static_cast<i32>(N)) ? static_cast<M>(0) : static_cast<M>(x >> shift);
	}
	else {
		return to_vector_<ISA, M, T>(mask_rshift_<ISA, N, T>(to_mask_<ISA, T>(x), shift));
	}
}

__RAZE_VX_NAMESPACE_END
