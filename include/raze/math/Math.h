#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/math/Abs.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>
#include <src/raze/math/Fnma.h>
#include <src/raze/math/Fnms.h>
#include <src/raze/math/Sin.h>
#include <src/raze/math/Cos.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <vx::arithmetic_type T1, vx::arithmetic_type T2>
raze_always_inline constexpr auto max(const T1& x, const T2& y) noexcept {
	return (x < y) ? y : x;
}

template <class T1, class T2>
raze_always_inline constexpr auto min(const T1& x, const T2& y) noexcept {
	return (x > y) ? y : x;
}

__RAZE_MATH_NAMESPACE_END
