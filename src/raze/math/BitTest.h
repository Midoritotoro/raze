#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::unsigned_integral T>
raze_always_inline bool bit_test(T x, i32 i) noexcept {
    raze_debug_assert(i < raze_sizeof_in_bits(T) && i >= 0);

//#if defined(raze_cpp_msvc_only)
//    if constexpr (sizeof(T) == 8) return static_cast<bool>(_bittest64(reinterpret_cast<const i64*>(&x), i));
//    else if constexpr (sizeof(T) == 4) return static_cast<bool>(_bittest(reinterpret_cast<const long32*>(&x), i));
//    else
//#endif
        return (x >> i) & 1;
}

__RAZE_MATH_NAMESPACE_END
