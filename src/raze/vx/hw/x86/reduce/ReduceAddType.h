#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_>
using __reduce_type = std::conditional_t<std::is_floating_point_v<_Type_>, f64,
    std::conditional_t<std::is_unsigned_v<_Type_>, u64, i64>>;

__RAZE_VX_NAMESPACE_END
