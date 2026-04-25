#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <concepts>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::integral _Type_>
constexpr auto sign_bit() noexcept {
    return static_cast<std::make_unsigned_t<_Type_>>(1) << (sizeof(_Type_) * CHAR_BIT - 1);
};

__RAZE_MATH_NAMESPACE_END
