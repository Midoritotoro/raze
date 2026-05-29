#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/shuffle/Shuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ __reverse(_Simd_ __x) noexcept {
	return __shuffle(__x, make_reversed_pattern<_Simd_>{});
}

__RAZE_VX_NAMESPACE_END
