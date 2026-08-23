#pragma once 

#include <src/raze/options/Or.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Condition>
struct if_ {
    static constexpr bool has_alternative = false;
    using condition_type = Condition;

    if_(const Condition& condition) noexcept:
        _condition(condition)
    {}

    raze_always_inline Condition mask() const noexcept {
        return _condition;
    }

    Condition _condition;
};

__RAZE_OPTIONS_NAMESPACE_END
