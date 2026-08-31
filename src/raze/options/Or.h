#pragma once 

#include <src/raze/options/Concepts.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Condition, class Value>
struct or_ {
    static constexpr bool has_alternative = true;

    using alternative_type = Value;
    using condition_type = Condition;

    constexpr or_(const Condition& condition, const Value& v) noexcept:
        _condition(condition), _alternative(v)
    {}

    constexpr or_(const or_&) noexcept = default;
    constexpr or_(or_&&) noexcept = default;

    constexpr or_& operator=(const or_&) noexcept = default;
    constexpr or_& operator=(or_&&) noexcept = default;

    constexpr raze_always_inline Value alternative() const noexcept {
        return _alternative;
    }

    raze_always_inline Condition mask() const noexcept {
        return _condition;
    }

    Condition _condition;
    Value _alternative;
};

__RAZE_OPTIONS_NAMESPACE_END