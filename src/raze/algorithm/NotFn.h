#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Fn_>
struct not_fn {
    _Fn_ _fn;

    constexpr explicit not_fn(_Fn_&& __fn) noexcept :
        _fn(std::move(__fn)) 
    {}
    
    template <class ... _Args_>
    raze_always_inline auto operator()(_Args_&& ... __args) const 
        noexcept(noexcept(_fn(std::forward<_Args_>(__args)...))) 
    {
        return _fn(std::forward<_Args_>(__args)...); 
    }
};

__RAZE_ALGORITHM_NAMESPACE_END
