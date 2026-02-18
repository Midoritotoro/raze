#pragma once 

#include <raze/Types.h>

__RAZE_MEMORY_NAMESPACE_BEGIN

template <class _Type_>
constexpr auto pointer_to_integral(_Type_ __pointer) noexcept {
    if      constexpr (std::is_same_v<std::decay_t<_Type_>, std::nullptr_t>)
        return 0;
    else if constexpr (std::is_pointer_v<std::decay_t<_Type_>>)
        return reinterpret_cast<uintptr>(__pointer);
    else
        return __pointer;
}

__RAZE_MEMORY_NAMESPACE_END
