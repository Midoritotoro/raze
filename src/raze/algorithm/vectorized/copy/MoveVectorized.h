#pragma once

#include <raze/vx/BasicSimd.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN


void* __memmove_vectorized(
    void*       __destination,
    const void* __source,
    sizetype    __bytes) noexcept
{
    return memmove(__destination, __source, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
