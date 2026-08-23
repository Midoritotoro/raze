#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Fetch.h>
#include <src/raze/options/ConditionalCallable.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class F, class ... Args>
constexpr raze_no_stack_protector raze_always_inline auto dispatch_call(const F& f, Args&& ... args) noexcept {
    using ReturnType = decltype(f(std::forward<Args>(args)...));

    if constexpr (std::is_void_v<ReturnType>) f.behavior(f.options(), std::forward<Args>(args)...);
    else return f.behavior(f.options(), std::forward<Args>(args)...);
}

__RAZE_OPTIONS_NAMESPACE_END
