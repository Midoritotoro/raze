#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <raze/compatibility/Nodiscard.h>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <class _Function_> 
constexpr inline bool __is_lightweight_callable_v = std::conjunction_v<
    std::bool_constant<sizeof(_Function_) <= sizeof(void*)>,
    std::is_trivially_copy_constructible<_Function_>,
    std::is_trivially_destructible<_Function_>>;

template <class _Function_>
struct __function_reference {
    _Function_& _function;
    using function_unwrapped_type = _Function_;

    constexpr raze_always_inline operator _Function_& () noexcept {
        return _function;
    }

    template <class ... _Args_>
    constexpr raze_always_inline decltype(auto) operator()(_Args_&& ... __values) const
        noexcept(std::is_nothrow_invocable_v<_Function_&, _Args_...>)
            requires (std::invocable<_Function_, _Args_...>)
    {
        if constexpr (std::is_member_pointer_v<_Function_>)
            return std::invoke(_function, std::forward<_Args_>(__values)...);
        else
            return _function(std::forward<_Args_>(__values)...);
    }
};

template <class _Function_>
raze_nodiscard raze_always_inline constexpr auto __pass_function(_Function_& __function) noexcept {
    if constexpr (__is_lightweight_callable_v<_Function_>) return __function;
    else return __function_reference<_Function_>(__function);
}

__RAZE_TYPE_TRAITS_NAMESPACE_END

