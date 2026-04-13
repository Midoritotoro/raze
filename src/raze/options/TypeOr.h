#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    concepts::keyword   _Type_,
    class               _Value_> 
struct __type_or {
    _Value_ _value;

    template <concepts::option ... _Options_>
    constexpr decltype(auto) operator()(_Options_&&... __options) const noexcept { 
        return fetch(*this, std::forward<_Options_>(__options)...); 
    }
};

__RAZE_OPTIONS_NAMESPACE_END
