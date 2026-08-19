#pragma once 

#include <src/raze/options/Concepts.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Condition_, class _Value_>
struct or_ {
    static constexpr bool has_alternative = true;

    using alternative_type = _Value_;
    using condition_type = _Condition_;

    constexpr or_(const _Condition_& __condition, const _Value_& __value) noexcept:
        _condition(__condition), _alternative(__value)
    {}

    constexpr or_(const or_&) noexcept = default;
    constexpr or_(or_&&) noexcept = default;

    constexpr or_& operator=(const or_&) noexcept = default;
    constexpr or_& operator=(or_&&) noexcept = default;

    constexpr raze_always_inline _Value_ alternative() const noexcept {
        return _alternative;
    }

    template <class _Type_>
    raze_always_inline _Condition_ mask(const as<_Type_>&) const noexcept {
        return _condition;
    }

    _Condition_ _condition;
    _Value_ _alternative;
};

__RAZE_OPTIONS_NAMESPACE_END