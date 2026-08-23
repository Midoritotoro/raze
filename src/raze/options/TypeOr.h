#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::keyword T, class Value> 
struct type_or {
    raze_no_unique_address Value _value;

    template <concepts::option ... Options>
    constexpr raze_always_inline decltype(auto) operator()(Options&&... opts) const noexcept {
        return fetch(*this, std::forward<Options>(opts)...); 
    }
};

__RAZE_OPTIONS_NAMESPACE_END
