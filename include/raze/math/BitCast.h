#pragma once

#include <raze/Types.h>
#include <src/raze/RazeNamespace.h>
#include <src/raze/compatibility/Inline.h>
#include <cstddef>
#include <type_traits>

__RAZE_MATH_NAMESPACE_BEGIN

template <class To, class From>
raze_always_inline constexpr To bit_cast(const From& from) noexcept
    requires ((sizeof(To) == sizeof(From)))
{
    return __builtin_bit_cast(To, from);
}

template <class T>
raze_always_inline constexpr auto pointer_to_integral(T ptr) noexcept {
    if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>) return static_cast<uintptr>(0);
    else if constexpr (std::is_pointer_v<std::decay_t<T>>) return reinterpret_cast<uintptr>(ptr);
    else return ptr;
}

__RAZE_MATH_NAMESPACE_END
