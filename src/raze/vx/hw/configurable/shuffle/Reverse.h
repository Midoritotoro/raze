#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_reverse_t: options::conditional_callable<configurable_reverse_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    raze_nodiscard static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        return reverse_(x, make_reversed_pattern<V>{});
    }
};

constexpr inline auto reverse = options::functor<configurable_reverse_t>;

__RAZE_VX_NAMESPACE_END
