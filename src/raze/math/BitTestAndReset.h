#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Compatibility.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <class _IntegralType_>
raze_always_inline bool __bit_test_and_reset(
    _IntegralType_& __integer,
    i32	        __bit_index) noexcept
{
    raze_debug_assert(__bit_index < raze_sizeof_in_bits(_IntegralType_) && __bit_index >= 0);

#if defined(raze_cpp_msvc)
    if constexpr (sizeof(_IntegralType_) == 8)
        return static_cast<bool>(_bittestandreset64(
            reinterpret_cast<i64*>(&__integer), __bit_index));
    else
        return static_cast<bool>(_bittestandreset(
            reinterpret_cast<long32*>(&__integer), __bit_index));

#elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
    bool __old_bit = 0;

    asm volatile(
        "btr %2, %1\n\t"
        "setc %0"
        : "=r"(__old_bit), "+m"(__integer)
        : "r"(__bit_index)
        : "cc"
        );

    return __old_bit;
#endif // defined(_MSC_VER)
}

__RAZE_MATH_NAMESPACE_END
