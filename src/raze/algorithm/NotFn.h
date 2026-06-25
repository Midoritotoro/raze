#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <concepts>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Fn_>
struct not_fn {
    _Fn_& _fn;
    using function_unwrapped_type = _Fn_;

    constexpr raze_always_inline operator _Fn_& () noexcept {
        return _fn;
    }

    template <class ... _Args_>
    raze_always_inline constexpr decltype(auto) operator()(_Args_&& ... __args) const 
        noexcept(noexcept(!_fn(std::forward<_Args_>(__args)...)))
            requires (std::invocable<_Fn_, _Args_...>)
    {
        return !_fn(std::forward<_Args_>(__args)...); 
    }
};

template <class _Fn_>
constexpr raze_always_inline decltype(auto) make_not_fn(_Fn_& __fn) noexcept {
    return not_fn<_Fn_>(__fn);
}

__RAZE_ALGORITHM_NAMESPACE_END
