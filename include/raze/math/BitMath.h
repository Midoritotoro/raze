#pragma once 

#include <src/raze/math/CountTrailingZeros.h>
#include <src/raze/math/CountLeadingZeros.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <typename _Type_>
constexpr raze_always_inline _Type_ clear_left_most_set(const _Type_ __value) {
    return __value & (__value - 1);
}

template <typename _IntegralType_>
constexpr raze_always_inline int count_trailing_zero_bits(_IntegralType_ __value) noexcept {
    return __count_trailing_zero_bits(__value);
}

template <typename _IntegralType_>
constexpr raze_always_inline int count_leading_zero_bits(_IntegralType_ __value) noexcept {
    return __count_leading_zero_bits(__value);
}
 
template <typename _IntegralType_>
constexpr raze_always_inline int population_count(_IntegralType_ __value) noexcept {
    return __population_count(__value);
}

__RAZE_MATH_NAMESPACE_END