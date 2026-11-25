#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/traits/FunctionPass.h>
#include <concepts>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Fn>
struct not_fn {
    static constexpr bool by_value = traits::lightweight_callable<Fn>;
    using storage_type = std::conditional_t<by_value, Fn, Fn&>;
    raze_no_unique_address storage_type _fn;
    using function_unwrapped_type = Fn;

    constexpr raze_always_inline explicit not_fn(Fn& fn) noexcept
        : _fn(fn)
    {}

    constexpr raze_always_inline operator Fn& () noexcept {
        return _fn;
    }

    template <class ... Args>
    raze_always_inline constexpr decltype(auto) operator()(Args&& ... args)
        noexcept(noexcept(!std::invoke(_fn, std::forward<Args>(args)...)))
            requires (std::invocable<Fn&, Args...>)
    {
        return !std::invoke(_fn, std::forward<Args>(args)...);
    }

    template <class ... Args>
    raze_always_inline constexpr decltype(auto) operator()(Args&& ... args) const 
        noexcept(noexcept(!std::invoke(_fn, std::forward<Args>(args)...)))
            requires (std::invocable<const storage_type&, Args...>)
    {
        return !std::invoke(_fn, std::forward<Args>(args)...); 
    }
};

template <class Fn>
constexpr raze_always_inline decltype(auto) make_not_fn(Fn& fn) noexcept(noexcept(not_fn<Fn>(fn))) {
    return not_fn<Fn>(fn);
}

__RAZE_ALGORITHM_NAMESPACE_END
