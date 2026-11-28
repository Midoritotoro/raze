#pragma once

#include <src/raze/options/Keywords.h>
#include <src/raze/options/Option.h>

__RAZE_VX_NAMESPACE_BEGIN

struct raw_mode {};

/**
 * @ingroup raze_vx_arith
 * @memberof raze::vx
 * @hideinitializer
 * @brief Arithmetic option selecting fast hardware reciprocal/rsqrt approximations (`rcpps`/`rsqrtps`/`rcp14`/`rsqrt14`).
 *
 * Trades full IEEE-754 precision for single-instruction hardware throughput (~12-bit precision on SSE/AVX,
 * ~14-bit precision on AVX-512).
 * @see rsqrt, rcp
*/
constexpr inline auto raw = raze::options::flag(raw_mode{});
struct raw_option : raze::options::exact_option<raw> {};

__RAZE_VX_NAMESPACE_END

