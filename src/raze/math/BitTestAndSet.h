#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <std::unsigned_integral T>
raze_always_inline bool bit_test_and_set(T& x, i32 i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T) && i >= 0);

//#if defined(raze_cpp_msvc_only)
//    if constexpr (sizeof(T) == 8) return static_cast<bool>(_bittestandset64(reinterpret_cast<i64*>(&x), i));
//    else if constexpr (sizeof(T) == 4) return static_cast<bool>(_bittestandset(reinterpret_cast<long32*>(&x), i));
//    else
//#endif
    {
        T mask = T{ 1 } << i;
        bool old = (x & mask) != T{ 0 };
        x |= mask;
        return old;
    }
}

__RAZE_MATH_NAMESPACE_END
