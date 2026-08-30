#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type _Type_, raw_mask_type M>
raze_always_inline M mask_lshift_(M x, i32 shift) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return x;
	}
	else if constexpr (std::is_integral_v<M>) {
		raze_maybe_unused_attribute constexpr auto all_mask = ((sizeof(M) * 8) == N)
			? math::max_limit<M>() : M(((M(1) << N) - 1));

		if constexpr (N < 8) return (x << shift) & all_mask;
		else return (x << shift);
	}
	else {
		
	}
}

__RAZE_VX_NAMESPACE_END
