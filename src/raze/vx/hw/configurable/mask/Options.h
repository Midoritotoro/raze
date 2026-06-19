#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

struct not_null_mode {};
constexpr inline auto not_null = raze::options::flag(not_null_mode{});
struct not_null_option : raze::options::exact_option<not_null> {};

__RAZE_VX_NAMESPACE_END
