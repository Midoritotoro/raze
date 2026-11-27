#pragma once 

#include <raze/Types.h>
#include <src/raze/utility/Assert.h>
#include <src/raze/compatibility/Compatibility.h>
#include <concepts>


__RAZE_MATH_NAMESPACE_BEGIN

template <std::unsigned_integral T>
raze_always_inline bool bit_test_and_reset(T& x, sizetype i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T));
    T mask = T(1) << i;
    bool old = (x & mask) != T(0);
    x &= static_cast<T>(~mask);
    return old;
}

__RAZE_MATH_NAMESPACE_END
