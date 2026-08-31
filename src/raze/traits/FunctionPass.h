#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <raze/compatibility/Nodiscard.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

template <class F> 
constexpr inline bool is_lightweight_callable_v = std::conjunction_v<std::bool_constant<sizeof(F) <= sizeof(void*)>,
    std::is_trivially_copy_constructible<F>, std::is_trivially_destructible<F>>;

template <class F>
struct function_reference {
    using function_unwrapped_type = F;

    constexpr raze_always_inline operator function_unwrapped_type&() noexcept {
        return _f;
    }

    template <class ... Args>
    constexpr raze_always_inline decltype(auto) operator()(Args&& ... args) const
        noexcept(std::is_nothrow_invocable_v<F, Args...>) requires(std::invocable<F, Args...>)
    {
        if constexpr (std::is_member_pointer_v<F>) return std::invoke(_f, std::forward<Args>(args)...);
        else return _f(std::forward<Args>(args)...);
    }

    F& _f;
};

template <class F>
concept has_function_unwrapped_type = requires {
    typename F::function_unwrapped_type;
};

template <class F>
struct function_unwrapped_impl {
    using type = F;
};

template <has_function_unwrapped_type F>
struct function_unwrapped_impl<F> {
    using type = typename F::function_unwrapped_type;
};

template <class F>
using function_unwrapped = typename function_unwrapped_impl<F>::type;

template <class F>
raze_always_inline constexpr function_unwrapped<F> unwrap_function(F&& f) noexcept {
    return static_cast<function_unwrapped<F>>(std::forward<F>(f));
}

template <class F>
raze_nodiscard raze_always_inline constexpr decltype(auto) fwd_fn(F& f) noexcept {
    if constexpr (has_function_unwrapped_type<F>) return f;
    else if constexpr (is_lightweight_callable_v<F>) return f;
    else return function_reference<F>(f);
}

__RAZE_TRAITS_NAMESPACE_END

