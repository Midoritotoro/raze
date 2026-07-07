#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/SFence.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_sfence: options::strict_elementwise_callable<_Configurable_sfence, _Options_> {
    raze_always_inline void operator()() const noexcept {
        return raze::options::__dispatch_call(*this);
    }

    static raze_always_inline auto deferred_call(auto __options) noexcept {
        return __x86_store_fence();
    }

    using callable_tag_type = _Configurable_sfence;
};

__RAZE_VX_NAMESPACE_END
