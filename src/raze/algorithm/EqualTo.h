#pragma once

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#define RAZE_DEFINE_COMPARATOR(__name, __operator)                  \
    template <class _Type_>                                         \
    struct __name {                                                 \
        _Type_ _v;                                                  \
                                                                    \
        constexpr explicit __name(_Type_ __v) noexcept              \
            : _v(std::move(__v))                                    \
        {}                                                          \
                                                                    \
        template <class _Other_>                                    \
        raze_always_inline constexpr auto operator()(               \
            const _Other_& __x) const                               \
            noexcept(noexcept(__x __operator _v))                   \
        {                                                           \
            return __x __operator _v;                               \
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