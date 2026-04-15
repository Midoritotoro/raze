#pragma once 

#include <raze/RazeNamespace.h>


__RAZE_VX_NAMESPACE_BEGIN

struct __aligned_policy {
    static constexpr bool __alignment = true;
};

struct __unaligned_policy {
    static constexpr bool __alignment = false;
};

__RAZE_VX_NAMESPACE_END
