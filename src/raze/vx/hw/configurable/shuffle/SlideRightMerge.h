#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/SlideRightMerge.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_slide_right_merge_t : options::conditional_callable<configurable_slide_right_merge_t, Options> {
    template <simd_type V, sizetype Elements>
    raze_nodiscard raze_always_inline V operator()(const V& x,
        const V& y, std::integral_constant<sizetype, Elements> i) const noexcept
    {
        return options::dispatch_call(*this, x, y, i);
    }

    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, const V& y, i32 i) const noexcept {
        return options::dispatch_call(*this, x, y, i);
    }

    template <simd_type V, sizetype Elements>
    static raze_always_inline auto deferred_call(auto opts, const V& x,
        const V& y, std::integral_constant<sizetype, Elements> i) noexcept 
    {
        return slide_right_merge_(x, y, i);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y, i32 i) noexcept {
        return slide_right_merge_(x, y, i);
    }
};

constexpr inline auto slide_right_merge = raze::options::functor<configurable_slide_right_merge_t>;

__RAZE_VX_NAMESPACE_END
