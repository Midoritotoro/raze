#pragma once

#include <raze/RazeNamespace.h>

#include <raze/compatibility/Inline.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <raze/compatibility/SimdCompatibility.h>
#include <raze/arch/ProcessorFeatures.h>

#include <src/raze/algorithm/AdvanceBytes.h>

#include <raze/math/BitMath.h>
#include <raze/compatibility/Inline.h>

#include <raze/datapar/BasicSimd.h>



__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline const void* _MaxElementScalar(
    const void* _First,
    const void* _Last) noexcept
{
    if (_First == _Last)
        return _Last;

    const _Type_* _FirstCasted = static_cast<const _Type_*>(_First);
    auto _Max = _FirstCasted;

    for (; ++_FirstCasted != _FirstCasted; )
        if (*_FirstCasted > *_Max)
            _Max = _FirstCasted;

    return _Max;
}

template <class _Type_>
raze_always_inline _Type_* _MaxElementVectorized(
    const void* __first,
    const void* __last) noexcept
{
    
}

__RAZE_ALGORITHM_NAMESPACE_END