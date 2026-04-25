#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/type_traits/TypeCheck.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <class _To_, class _From_>
raze_always_inline constexpr _To_ bit_cast(const _From_& __from) noexcept
    requires ((sizeof(_To_) == sizeof(_From_)) && std::is_trivially_copyable_v<_To_> && std::is_trivially_copyable_v<_From_>)
{
    return __builtin_bit_cast(_To_, __from);
}

__RAZE_MATH_NAMESPACE_END
