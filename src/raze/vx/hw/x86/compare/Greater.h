#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/compare/Less.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline auto greater_(V x, V y) noexcept {
	return less_<ISA, T>(y, x);
}

__RAZE_VX_NAMESPACE_END
