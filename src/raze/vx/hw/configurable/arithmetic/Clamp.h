#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_clamp : raze::options::strict_elementwise_callable<_Configurable_clamp, _Options_> {
    template <simd_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __v, const _Type_& __low, const _Type_& __high) const noexcept {
        return raze::options::__dispatch_call(*this, __v, __low, __high);
    }

    template <simd_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __v, const _Type_& __low, const _Type_& __high) noexcept {
        return __vmin[__options](__vmax[__options](__v, __low), __high);
    }

    using callable_tag_type = _Configurable_clamp;
};

__RAZE_VX_NAMESPACE_END
