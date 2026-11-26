#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type _Type_, raw_mask_type M>
raze_always_inline M mask_lshift_(M x, i32 shift) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return (shift == 0) ? x : false;
	}
	else if constexpr (std::is_integral_v<M>) {
		if (shift >= static_cast<i32>(N))
			return static_cast<M>(0);

		if constexpr (N < sizeof(M) * 8) {
			constexpr auto all_mask = static_cast<M>((M(1) << N) - 1);
			return static_cast<M>((x << shift) & all_mask);
		}
		else {
			return static_cast<M>(x << shift);
		}
	}
	else {
		return to_vector_<ISA, M, _Type_>(mask_lshift_<ISA, N, _Type_>(to_mask_<ISA, _Type_>(x), shift));
	}
}

__RAZE_VX_NAMESPACE_END
