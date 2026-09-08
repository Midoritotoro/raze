#pragma once 

#include <raze/Types.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class V, class T>
struct insert_mask {
    T array[(sizeof(V) / sizeof(T)) << 1];
    i32 offset = 0;
};

template <class V, class T>
constexpr raze_always_inline auto make_insert_mask() noexcept {
    constexpr auto length = (sizeof(V) / sizeof(T)) << 1;
    auto mask = insert_mask<V, T>();

    for (auto i = 0; i < length; ++i)
        mask.array[i] = 0;

    mask.offset = length >> 1;
    mask.array[mask.offset] = -1;

    return mask;
}

__RAZE_VX_NAMESPACE_END
