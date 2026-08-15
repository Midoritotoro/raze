#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Fetch.h>
#include <src/raze/options/ConditionalCallable.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Callable_, class ... _Args_>
constexpr raze_no_stack_protector raze_always_inline auto __dispatch_call(
    const _Callable_& __callable, _Args_&& ... __args) noexcept 
{
    using _ReturnType = decltype(__callable(std::forward<_Args_>(__args)...));

    if constexpr (std::is_void_v<_ReturnType>) __callable.behavior(__callable.options(), std::forward<_Args_>(__args)...);
    else return __callable.behavior(__callable.options(), std::forward<_Args_>(__args)...);
}

__RAZE_OPTIONS_NAMESPACE_END
