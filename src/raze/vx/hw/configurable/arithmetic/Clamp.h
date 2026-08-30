#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_clamp_t : options::conditional_callable<configurable_clamp_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& v, const V& low, const V& high) const noexcept {
        return options::dispatch_call(*this, v, low, high);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& v, const V& low, const V& high) noexcept {
        return vmin[opts](vmax[opts](v, low), high);
    }
};

constexpr inline auto clamp = options::functor<configurable_clamp_t>;

__RAZE_VX_NAMESPACE_END
