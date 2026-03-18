#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>


__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline float __cotan(float __x) noexcept {

}

constexpr raze_always_inline double __cotan(double __x) noexcept {

}

template <class _Simd_>
raze_always_inline _Simd_ __cotan(const _Simd_& __x) noexcept
    requires(datapar::__is_valid_simd_v<_Simd_>&& std::is_floating_point_v<typename _Simd_::value_type>)
{
    using _ValueType = typename _Simd_::value_type;

    if constexpr (std::is_same_v<_ValueType, float>) {

    }
    else if constexpr (std::is_same_v<_ValueType, double>) {

    }
}

__RAZE_MATH_NAMESPACE_END
