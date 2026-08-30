#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/SFence.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_sfence_t: options::conditional_callable<configurable_sfence_t, Options> {
    raze_always_inline void operator()() const noexcept {
        return options::dispatch_call(*this);
    }

    static raze_always_inline auto deferred_call(auto) noexcept {
        return x86_store_fence_();
    }
};

constexpr inline auto sfence = options::functor<configurable_sfence_t>;

__RAZE_VX_NAMESPACE_END
