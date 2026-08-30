#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_type V>
raze_always_inline auto to_bitmask_(V x) noexcept {
	if constexpr (!has_avx512bw<ISA> && sizeof(T) == 2) return to_mask_<ISA, i8>(x);
	else return to_mask_<ISA, T>(x);
}

template <arch::ISA	ISA, arithmetic_type T, std::unsigned_integral M>
raze_always_inline auto to_bitmask_(M x) noexcept {
	return x;
}

__RAZE_VX_NAMESPACE_END
