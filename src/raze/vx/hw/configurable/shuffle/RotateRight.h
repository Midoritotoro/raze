#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/RotateRight.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct configurable_rotate_right_t : options::conditional_callable<configurable_rotate_right_t, Options> {
    template <simd_type V, sizetype _Elements_>
    raze_always_inline _Type_ operator()(const _Type_& x, std::integral_constant<sizetype, _Elements_> i) const noexcept {
        return options::dispatch_call(*this, x, i);
    }

    template <simd_type V>
    raze_always_inline V operator()(const V& x, i32 i) const noexcept {
        return options::dispatch_call(*this, x, i);
    }

    template <simd_type V, sizetype _Elements_>
    static raze_always_inline auto deferred_call(auto opts, const V& x,
        std::integral_constant<sizetype, Elements> i) noexcept 
    {
        return rotate_right_(x, make_rotate_right_pattern<V, i>{});
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, i32 i) noexcept {
        return rotate_right_(x, i);
    }
};

constexpr inline auto rotate_right = options::functor<configurable_rotate_right_t>;

__RAZE_VX_NAMESPACE_END
