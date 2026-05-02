#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>


__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline f32 __cotan(f32 __x) noexcept {

}

constexpr raze_always_inline f64 __cotan(f64 __x) noexcept {

}

template <class _Simd_>
raze_always_inline _Simd_ __cotan(const _Simd_& __x) noexcept
    requires(vx::__is_valid_simd_v<_Simd_>&& std::is_floating_point_v<typename _Simd_::value_type>)
{
    using _ValueType = typename _Simd_::value_type;

    if constexpr (std::is_same_v<_ValueType, f32>) {

    }
    else if constexpr (std::is_same_v<_ValueType, f64>) {

    }
}

__RAZE_MATH_NAMESPACE_END
