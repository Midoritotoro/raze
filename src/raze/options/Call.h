#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class F>
struct call {
    constexpr call(F f) noexcept :
        _callable(f)
    {}

    constexpr raze_always_inline auto perform() const
        noexcept(std::is_nothrow_invocable_v<F>) 
    {
        return _callable();
    }

    raze_no_unique_address F _callable;
};

__RAZE_OPTIONS_NAMESPACE_END