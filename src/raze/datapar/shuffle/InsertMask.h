#pragma once 

#include <raze/Types.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _VectorType_,
    class _DesiredType_>
struct __insert_mask {
    _DesiredType_   __array[(sizeof(_VectorType_) / sizeof(_DesiredType_)) << 1];
    int32           __offset = 0;
};

template <
    class _VectorType_,
    class _DesiredType_>
constexpr auto __simd_make_insert_mask() noexcept {
    constexpr auto __length = (sizeof(_VectorType_) / sizeof(_DesiredType_)) << 1;
    auto __mask = __insert_mask<_VectorType_, _DesiredType_>();

    for (auto __index = 0; __index < __length; ++__index)
        __mask.__array[__index] = 0;

    __mask.__offset = __length >> 1;
    __mask.__array[(__mask.__offset + 1)] = -1;

    return __mask;
}

__RAZE_DATAPAR_NAMESPACE_END
