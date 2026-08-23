#pragma once 

#include <src/raze/math/CountTrailingZeros.h>
#include <src/raze/math/CountLeadingZeros.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <class T>
constexpr raze_always_inline T clear_left_most_set(T v) {
    return v & (v - 1);
}

template <class T>
constexpr raze_always_inline i32 count_trailing_zero_bits(T v) noexcept {
    return count_trailing_zero_bits_impl(v);
}

template <class T>
constexpr raze_always_inline i32 count_leading_zero_bits(T v) noexcept {
    return count_leading_zero_bits_impl(v);
}
 
template <class T>
constexpr raze_always_inline i32 population_count(T v) noexcept {
    return population_count_impl(v);
}

__RAZE_MATH_NAMESPACE_END