#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>


__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline float __sin(float __x) noexcept {
    constexpr auto __sine_table_size = 256;

    const auto __si = int(__x * (0.5f * __sine_table_size / M_PI)) & (__sine_table_size - 1);
    const auto  __ci = int(__si + __sine_table_size / 4) & (__sine_table_size - 1);

    const auto __d = float(__x - __si * (2.0f * M_PI / __sine_table_size));

    return float(__sine_table[__si] + (__sine_table[__ci] - 0.5f * __sine_table[__si] * __d) * __d);
}

constexpr raze_always_inline double __sin(double __x) noexcept {
    constexpr auto __sine_table_size = 256;

    const auto __si = int(__x * (0.5 * __sine_table_size / M_PI)) & (__sine_table_size - 1);
    const auto  __ci = int(__si + __sine_table_size / 4) & (__sine_table_size - 1);

    const auto __d = float(__x - __si * (2.0 * M_PI / __sine_table_size));

    return __sine_table[__si] + (__sine_table[__ci] - 0.5 * __sine_table[__si] * __d) * __d;
}

template <class _Simd_>
raze_always_inline _Simd_ __sin(const _Simd_& __x) noexcept
	requires(datapar::__is_valid_simd_v<_Simd_> && std::is_floating_point_v<typename _Simd_::value_type>)
{
    using _ValueType = typename _Simd_::value_type; 

    if constexpr (std::is_same_v<_ValueType, float>) {
        const auto __x2 = __x * __x;

        _Simd_ __y = -0x1.9CC000p-13f;      // -1/7!
        __y = __y * __x2 + 0x1.111100p-7f;  //  1/5!
        __y = __y * __x2 - 0x1.555556p-3f;  // -1/3!

        return __y * (__x2 * __x) + __x;
    }
    else if constexpr (std::is_same_v<_ValueType, double>) {
        const auto __x2 = __x * __x;

        _Simd_ __y = -0x1.ACF0000000000p-41;        // -1/15!
        __y = __y * __x2 + 0x1.6124400000000p-33;   //  1/13!
        __y = __y * __x2 - 0x1.AE64567000000p-26;   // -1/11!
        __y = __y * __x2 + 0x1.71DE3A5540000p-19;   //  1/9!
        __y = __y * __x2 - 0x1.A01A01A01A000p-13;   // -1/7!
        __y = __y * __x2 + 0x1.1111111111110p-7;    //  1/5!
        __y = __y * __x2 - 0x1.5555555555555p-3;    // -1/3!

        return __y * (__x2 * __x) + __x;
    }
}

__RAZE_MATH_NAMESPACE_END
