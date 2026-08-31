#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <class T>
raze_always_inline bool bit_test_and_set(T& x, i32 i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T) && i >= 0);

#if defined(raze_cpp_msvc)
    if constexpr (sizeof(T) == 8) return static_cast<bool>(_bittestandset64(reinterpret_cast<i64*>(&x), i));
    else return static_cast<bool>(_bittestandset(reinterpret_cast<long32*>(&x), i));
#elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
    bool old = 0;

    asm volatile(
        "bts %2, %1\n\t"
        "setc %0"
        : "=r"(old), "+m"(x)
        : "r"(i)
        : "cc"
    );

    return old;
#endif
}

__RAZE_MATH_NAMESPACE_END
