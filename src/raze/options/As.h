#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class T>
struct as {
    using type = T;

    constexpr as() noexcept {}
    explicit constexpr as(const T&)  noexcept {}
};

template <>
struct as<void> {
    using type = void;
    constexpr as() noexcept {}
};

__RAZE_OPTIONS_NAMESPACE_END
