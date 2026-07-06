#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Function_>
struct call {
    constexpr call(_Function_ __function) noexcept :
        _callable(__function)
    {}

    constexpr raze_always_inline auto perform() const
        noexcept(std::is_nothrow_invocable_v<_Function_>) 
    {
        return _callable();
    }

    raze_no_unique_address _Function_ _callable;
};

__RAZE_OPTIONS_NAMESPACE_END