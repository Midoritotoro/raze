#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <raze/compatibility/Nodiscard.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

template <class Function> 
constexpr inline bool is_lightweight_callable_v = std::conjunction_v<std::bool_constant<sizeof(Function) <= sizeof(void*)>,
    std::is_trivially_copy_constructible<Function>, std::is_trivially_destructible<Function>>;

template <class Function>
struct function_reference {
    using function_unwrapped_type = Function;

    constexpr raze_always_inline operator function_unwrapped_type&() noexcept {
        return _function;
    }

    template <class ... Args>
    constexpr raze_always_inline decltype(auto) operator()(Args&& ... args) const
        noexcept(std::is_nothrow_invocable_v<Function&, Args...>) requires(std::invocable<Function, Args...>)
    {
        if constexpr (std::is_member_pointer_v<Function>) return std::invoke(_function, std::forward<Args>(args)...);
        else return _function(std::forward<Args>(args)...);
    }

    Function& _function;
};

template <class Function>
concept has_function_unwrapped_type = requires {
    typename Function::function_unwrapped_type;
};

template <class Function>
struct function_unwrapped_impl {
    using type = Function;
};

template <has_function_unwrapped_type Function>
struct function_unwrapped_impl<Function> {
    using type = typename Function::function_unwrapped_type;
};

template <class Function>
using function_unwrapped = typename function_unwrapped_impl<Function>::type;

template <class Function>
raze_always_inline constexpr function_unwrapped<Function> unwrap_function(Function&& f) noexcept {
    return static_cast<function_unwrapped<Function>>(std::forward<Function>(f));
}

template <class Function>
raze_nodiscard raze_always_inline constexpr decltype(auto) fwd_fn(_Function_& function) noexcept {
    if constexpr (has_function_unwrapped_type<Function>) return function;
    else if constexpr (is_lightweight_callable_v<Function>) return function;
    else return function_reference<Function>(function);
}

__RAZE_TRAITS_NAMESPACE_END

