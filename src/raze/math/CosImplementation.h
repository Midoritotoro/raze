#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>


__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline float __cos(float __x) noexcept {
    constexpr auto __sine_table_size = 256;

    const auto __ci = int(__x * (0.5f * __sine_table_size / M_PI)) & (__sine_table_size - 1);
    const auto __si = int(__ci + __sine_table_size / 4) & (__sine_table_size - 1);

    const auto __d = __x - __ci * (2.0f * M_PI / __sine_table_size);

    return __sine_table[__si] - (__sine_table[__ci] + 0.5f * __sine_table[__si] * __d) * __d;
}

constexpr raze_always_inline double __cos(double __x) noexcept {
    constexpr auto __sine_table_size = 256;

    const auto __ci = int(__x * (0.5 * __sine_table_size / M_PI)) & (__sine_table_size - 1);
    const auto __si = int(__ci + __sine_table_size / 4) & (__sine_table_size - 1);

    const auto __d = __x - __ci * (2.0 * M_PI / __sine_table_size);

    return __sine_table[__si] - (__sine_table[__ci] + 0.5 * __sine_table[__si] * __d) * __d;
}

template <class _Simd_>
raze_always_inline _Simd_ __cos(const _Simd_& __x) noexcept
    requires(datapar::__is_valid_simd_v<_Simd_>&& std::is_floating_point_v<typename _Simd_::value_type>)
{
    using _ValueType = typename _Simd_::value_type;

    if constexpr (std::is_same_v<_ValueType, float>) {
        const auto __x2 = __x * __x;

        _Simd_ __y = 0x1.ap-16f;            //  1/8!
        __y = __y * __x2 - 0x1.6c1p-10f;    // -1/6!
        __y = __y * __x2 + 0x1.555556p-5f;  //  1/4!

        return __y * (__x2 * __x2) - .5f * __x2 + 1.f;
    }
    else if constexpr (std::is_same_v<_ValueType, double>) {
        const auto __x2 = __x * __x;

        _Simd_ __y = 0x1.AC00000000000p-45;         //  1/16!
        __y = __y * __x2 - 0x1.9394000000000p-37;   // -1/14!
        __y = __y * __x2 + 0x1.1EED8C0000000p-29;   //  1/12!
        __y = __y * __x2 - 0x1.27E4FB7400000p-22;   // -1/10!
        __y = __y * __x2 + 0x1.A01A01A018000p-16;   //  1/8!
        __y = __y * __x2 - 0x1.6C16C16C16C00p-10;   // -1/6!
        __y = __y * __x2 + 0x1.5555555555554p-5;    //  1/4!

        return (__y * __x2 - .5f) * __x2 + 1.f;
    }
}

__RAZE_MATH_NAMESPACE_END
