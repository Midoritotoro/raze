#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <raze/compatibility/Nodiscard.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

template <class _Function_> 
constexpr inline bool __is_lightweight_callable_v = std::conjunction_v<std::bool_constant<sizeof(_Function_) <= sizeof(void*)>,
    std::is_trivially_copy_constructible<_Function_>, std::is_trivially_destructible<_Function_>>;

template <class _Function_>
struct __function_reference {
    using function_unwrapped_type = _Function_;

    constexpr raze_always_inline operator _Function_&() noexcept {
        return _function;
    }

    template <class ... _Args_>
    constexpr raze_always_inline decltype(auto) operator()(_Args_&& ... __values) const
        noexcept(std::is_nothrow_invocable_v<_Function_&, _Args_...>) requires(std::invocable<_Function_, _Args_...>)
    {
        if constexpr (std::is_member_pointer_v<_Function_>) return std::invoke(_function, std::forward<_Args_>(__values)...);
        else return _function(std::forward<_Args_>(__values)...);
    }

    _Function_& _function;
};

template <class _Function_>
concept __has_function_unwrapped_type = requires {
    typename _Function_::function_unwrapped_type;
};

template <class _Function_>
struct __function_unwrapped_impl {
    using type = _Function_;
};

template <__has_function_unwrapped_type _Function_>
struct __function_unwrapped_impl<_Function_> {
    using type = typename _Function_::function_unwrapped_type;
};

template <class _Function_>
using __function_unwrapped = typename __function_unwrapped_impl<_Function_>::type;

template <class _Function_>
raze_always_inline constexpr __function_unwrapped<_Function_> __unwrap_function(_Function_&& __f) noexcept {
    return static_cast<__function_unwrapped<_Function_>>(std::forward<_Function_>(__f));
}

template <class _Function_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __fwd_fn(_Function_& __function) noexcept {
    if constexpr (__has_function_unwrapped_type<_Function_>) return __function;
    else if constexpr (__is_lightweight_callable_v<_Function_>) return __function;
    else return __function_reference<_Function_>(__function);
}

__RAZE_TRAITS_NAMESPACE_END

