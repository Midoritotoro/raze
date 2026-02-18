#pragma once

#include <src/raze/algorithm/vectorized/copy/MoveVectorized.h>

#include <raze/memory/Intersects.h>
#include <raze/memory/Alignment.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

void* __memcpy_vectorized(
    void*       __destination,
    const void* __source,
    sizetype    __bytes) noexcept
{
    return memcpy(__destination, __source, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END

