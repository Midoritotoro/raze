#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <std::unsigned_integral T>
raze_always_inline bool bit_test_and_set(T& x, sizetype i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T));
    T mask = T(1) << i;
    bool old = (x & mask) != T(0);
    x |= mask;
    return old;
}

__RAZE_MATH_NAMESPACE_END
