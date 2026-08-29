#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T>
using reduce_type = std::conditional_t<std::is_floating_point_v<T>, f64,
    std::conditional_t<std::is_unsigned_v<T>, u64, i64>>;

__RAZE_VX_NAMESPACE_END
