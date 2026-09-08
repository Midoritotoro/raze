#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <concepts>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::integral T>
raze_always_inline constexpr auto sign_bit() noexcept {
    return static_cast<std::make_unsigned_t<T>>(1) << (sizeof(T) * CHAR_BIT - 1);
};

__RAZE_MATH_NAMESPACE_END
