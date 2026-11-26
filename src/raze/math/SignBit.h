#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <concepts>
#include <limits.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::integral T>
raze_always_inline constexpr auto sign_bit() noexcept {
    if constexpr (std::same_as<T, bool>) {
        return true;
    }
    else {
        using U = std::make_unsigned_t<T>;
        return static_cast<U>(static_cast<U>(1) << (sizeof(T) * CHAR_BIT - 1));
    }
}

__RAZE_MATH_NAMESPACE_END
