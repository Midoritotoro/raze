#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::unsigned_integral T>
raze_always_inline bool bit_test(T x, sizetype i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T));
    return (x >> i) & 1;
}

__RAZE_MATH_NAMESPACE_END
