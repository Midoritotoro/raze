#pragma once 

#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Compact.h>
#include <src/raze/vx/hw/Compare.h>
#include <src/raze/vx/hw/Construct.h>
#include <src/raze/vx/hw/Shuffle.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>

__RAZE_VX_NAMESPACE_BEGIN

constexpr inline auto add = raze::options::functor<_Configurable_add>;
constexpr inline auto sub = raze::options::functor<_Configurable_sub>;
constexpr inline auto mul = raze::options::functor<_Configurable_mul>;
constexpr inline auto div = raze::options::functor<_Configurable_div>;
constexpr inline auto vertical_max = raze::options::functor<_Configurable_vertical_max>;
constexpr inline auto vertical_min = raze::options::functor<_Configurable_vertical_min>;
constexpr inline auto neg = raze::options::functor<_Configurable_neg>;
constexpr inline auto abs = raze::options::functor<_Configurable_abs>;

constexpr inline auto bit_or = raze::options::functor<_Configurable_or>;
constexpr inline auto bit_xor = raze::options::functor<_Configurable_xor>;
constexpr inline auto bit_and = raze::options::functor<_Configurable_and>;
constexpr inline auto bit_andnot = raze::options::functor<_Configurable_andnot>;
constexpr inline auto bit_shl = raze::options::functor<_Configurable_shl>;
constexpr inline auto bit_shr = raze::options::functor<_Configurable_shr>;
constexpr inline auto bit_not = raze::options::functor<_Configurable_not>;
//
//constexpr inline auto horizontal_sum = raze::options::functor<_Configurable_horizontal_sum>;
//constexpr inline auto horizontal_min = raze::options::functor<_Configurable_horizontal_min>;
//constexpr inline auto horizontal_max = raze::options::functor<_Configurable_horizontal_max>;
//
constexpr inline auto vmin = vertical_min;
constexpr inline auto vmax = vertical_max;
//constexpr inline auto hsum = horizontal_sum;
//constexpr inline auto hmin = horizontal_min;
//constexpr inline auto hmax = horizontal_max;

__RAZE_VX_NAMESPACE_END
