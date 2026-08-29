#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V>
raze_always_inline V all_ones_() noexcept {
	if constexpr (has_avx512f<ISA>) return broadcast_<ISA, V>(-1);
	else return equal_<ISA, i32>(zero_<ISA, V>(), zero_<ISA, V>());
}

__RAZE_VX_NAMESPACE_END
