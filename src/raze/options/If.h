#pragma once 

#include <src/raze/options/Or.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Condition_>
struct if_ {
    static constexpr bool has_alternative = false;
    using condition_type = _Condition_;

    if_(const _Condition_& __condition) noexcept:
        _condition(__condition)
    {}

    template <class _Value_>
    raze_always_inline auto else_(const _Value_& __value) const noexcept {
        return or_(*this, __value);
    }

    template <class _Type_>
    raze_always_inline _Condition_ mask(const as<_Type_>&) const noexcept {
        return _condition;
    }

    _Condition_ _condition;
};

__RAZE_OPTIONS_NAMESPACE_END
