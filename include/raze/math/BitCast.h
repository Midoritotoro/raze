#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/type_traits/TypeCheck.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <class _To_, class _From_>
raze_always_inline constexpr _To_ bit_cast(const _From_& __from) noexcept
    requires ((sizeof(_To_) == sizeof(_From_)))
{
    return __builtin_bit_cast(_To_, __from);
}

template <class _Type_>
raze_always_inline constexpr auto pointer_to_integral(_Type_ __pointer) noexcept {
    if constexpr (std::is_same_v<std::decay_t<_Type_>, std::nullptr_t>) return 0;
    else if constexpr (std::is_pointer_v<std::decay_t<_Type_>>) return reinterpret_cast<uintptr>(__pointer);
    else return __pointer;
}

__RAZE_MATH_NAMESPACE_END
