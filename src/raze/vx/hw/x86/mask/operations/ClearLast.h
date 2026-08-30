#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/math/SignBit.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, sizetype N, bool Unsafe, raw_mask_type M>
raze_always_inline M clear_first_(M x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return 0;
	}
	else if constexpr (std::is_integral_v<M>) {
		if constexpr (has_avx2<ISA>) {
			if constexpr (N == 64) return _blsr_u64(x);
			else return _blsr_u32(x);
		}
		else return x & (x - 1);
	}
	else if constexpr (intrin_type<M>) {
		return to_vector_<ISA, M, T>(clear_first_<ISA, T, N, Unsafe>(to_mask_<ISA, T>(x)));
	}
}

__RAZE_VX_NAMESPACE_END
