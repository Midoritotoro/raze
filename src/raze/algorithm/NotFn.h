#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <concepts>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Fn>
struct not_fn {
    Fn& _fn;
    using function_unwrapped_type = Fn;

    constexpr raze_always_inline operator Fn& () noexcept {
        return _fn;
    }

    template <class ... Args>
    raze_always_inline constexpr decltype(auto) operator()(Args&& ... args) const 
        noexcept(noexcept(!_fn(std::forward<Args>(args)...)))
            requires (std::invocable<Fn, Args...>)
    {
        return !_fn(std::forward<Args>(args)...); 
    }
};

template <class Fn>
constexpr raze_always_inline decltype(auto) make_not_fn(Fn& fn) noexcept {
    return not_fn<Fn>(fn);
}

__RAZE_ALGORITHM_NAMESPACE_END
