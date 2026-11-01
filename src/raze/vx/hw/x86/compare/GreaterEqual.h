#pragma once 

#include <src/raze/vx/hw/x86/compare/LessEqual.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto greater_equal_(V x, V y) noexcept {
	return less_equal_<ISA, T>(y, x);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_always_inline auto greater_equal_(V x, V y, Mask mask) noexcept {
	return less_equal_<ISA, T>(y, x, mask);
}

__RAZE_VX_NAMESPACE_END
