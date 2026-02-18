#pragma once

#include <src/raze/algorithm/AdvanceBytes.h>
#include <raze/compatibility/SimdCompatibility.h>

#include <raze/compatibility/FunctionAttributes.h>
#include <raze/arch/ProcessorFeatures.h>

#include <src/raze/type_traits/SimdTypeCheck.h>
#include <raze/datapar/BasicSimd.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN


void* __memmove_vectorized(
    void*       __destination,
    const void* __source,
    sizetype    __bytes) noexcept
{
    return memmove(__destination, __source, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
