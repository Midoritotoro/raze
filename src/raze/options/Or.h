#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    class _Condition_,
    class _Value_>
struct or_: _Condition_ {
    static constexpr bool has_alternative = true;

    using alternative_type = _Value_;
    using conditional_type = _Condition_;

    constexpr or_(const _Condition_& __condition, const _Value_& __value) noexcept: 
        _Condition_(__condition), _alternative(__value) 
    {}

    constexpr raze_always_inline _Condition_ base() const {
        return *this; 
    }

    template <class _Type_> 
    constexpr raze_always_inline auto rebase(_Type_ __value) const noexcept {
        return or_<_Condition_, _Type_>{static_cast<const _Condition_&>(*this), __value};
    }

    constexpr raze_always_inline _Condition_ drop_alternative() const noexcept {
        return *this; 
    }

    constexpr raze_always_inline _Value_ alternative() const noexcept {
        return _alternative;
    }

    constexpr raze_always_inline auto map_alternative(auto __operation) const noexcept(noexcept(__operation(_alternative))) {
        auto __mapped = __operation(_alternative);
        _Condition_ __condition = *this;

        return or_<_Condition_, decltype(__mapped)>{__condition, __mapped};
    }

    _Value_ _alternative;
};

__RAZE_OPTIONS_NAMESPACE_END
