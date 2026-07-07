#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

struct aligned_mode {};
constexpr inline auto aligned = raze::options::flag(aligned_mode{});
struct aligned_option : raze::options::exact_option<aligned> {};

struct nt_mode {};
constexpr inline auto nt = raze::options::flag(nt_mode{});
struct nt_option : raze::options::exact_option<nt> {};

__RAZE_VX_NAMESPACE_END
