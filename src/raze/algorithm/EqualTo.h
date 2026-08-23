#pragma once

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#define RAZE_DEFINE_COMPARATOR(name, op)                  \
    template <class T>                                         \
    struct name {                                                 \
        T _v;                                                  \
                                                                    \
        constexpr explicit __name(T v) noexcept              \
            : _v(std::move(v))                                    \
        {}                                                          \
                                                                    \
        template <class Other>                                    \
        raze_always_inline constexpr auto operator()(               \
            const Other& x) const                               \
            noexcept(noexcept(x op _v))                   \
        {                                                           \
            return x op _v;                               \
        }                                                           \
    }

RAZE_DEFINE_COMPARATOR(equal_to, ==);
RAZE_DEFINE_COMPARATOR(not_equal_to, !=);
RAZE_DEFINE_COMPARATOR(less, <);
RAZE_DEFINE_COMPARATOR(greater, >);
RAZE_DEFINE_COMPARATOR(less_equal, <=);
RAZE_DEFINE_COMPARATOR(greater_equal, >=);

#undef RAZE_DEFINE_COMPARATOR

__RAZE_ALGORITHM_NAMESPACE_END