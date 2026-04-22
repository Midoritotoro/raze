#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>
#include <src/raze/math/Fnma.h>
#include <src/raze/math/Fnms.h>
#include <src/raze/math/Sin.h>
#include <src/raze/math/Cos.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ sin(_Type_ __x) noexcept {
	return __sin(__x);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ cos(_Type_ __x) noexcept {
	return __cos(__x);
}

__RAZE_MATH_NAMESPACE_END
