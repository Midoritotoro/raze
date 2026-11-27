#pragma once 

#include <src/raze/compatibility/Compatibility.h>


__RAZE_NAMESPACE_BEGIN

struct uninitialized_tag {};
constexpr inline auto uninitialized = uninitialized_tag{};

__RAZE_NAMESPACE_END
