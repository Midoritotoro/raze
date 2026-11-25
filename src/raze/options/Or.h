#pragma once 

#include <src/raze/options/Concepts.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Condition, class Value>
struct or_ {
    static constexpr bool has_alternative = true;

    using alternative_type = Value;
    using condition_type = Condition;

    constexpr or_(const Condition& c, const Value& v) noexcept:
        _cond(c), _alternative(v)
    {}

    constexpr or_(const or_&) noexcept = default;
    constexpr or_(or_&&) noexcept = default;

    constexpr or_& operator=(const or_&) noexcept = default;
    constexpr or_& operator=(or_&&) noexcept = default;

    constexpr raze_always_inline const Value& alternative() const noexcept {
        return _alternative;
    }

    constexpr raze_always_inline const Condition& mask() const noexcept {
        return _cond;
    }

    Condition _cond;
    Value _alternative;
};

__RAZE_OPTIONS_NAMESPACE_END