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

constexpr inline auto abs = raze::options::functor<_Configurable_abs>;
constexpr inline auto fma = __fma;
constexpr inline auto fms = __fms;
constexpr inline auto fnma = __fnma;
constexpr inline auto fnms = __fnms;
constexpr inline auto sin = raze::options::functor<_Configurable_sin>;
constexpr inline auto cos = raze::options::functor<_Configurable_cos>;

__RAZE_MATH_NAMESPACE_END
