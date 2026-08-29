#pragma once 

#include <src/raze/vx/hw/x86/compare/LessEqual.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto greater_equal_(V x, V y) noexcept {
	return less_equal_<ISA, T>(y, x);
}

__RAZE_VX_NAMESPACE_END
