#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/RotateLeft.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_rotate_left : raze::options::strict_elementwise_callable<_Configurable_rotate_left, _Options_> {
    template <simd_type _Type_, sizetype _Elements_>
    raze_nodiscard raze_no_stack_protector raze_always_inline _Type_ operator()(const _Type_& __x, std::integral_constant<sizetype, _Elements_> __i) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __i);
    }

    template <simd_type _Type_>
    raze_nodiscard raze_no_stack_protector raze_always_inline _Type_ operator()(const _Type_& __x, i32 __i) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __i);
    }

    template <simd_type _Type_, sizetype _Elements_>
    raze_nodiscard static raze_no_stack_protector raze_always_inline auto deferred_call(auto __options, const _Type_& __x,
        std::integral_constant<sizetype, _Elements_> __i) noexcept 
    {
        return __rotate_left(__x, make_rotate_left_pattern<_Type_, __i>{});
    }

    template <simd_type _Type_>
    raze_nodiscard static raze_no_stack_protector raze_always_inline auto deferred_call(auto __options, const _Type_& __x, i32 __i) noexcept {
        return __rotate_left(__x, __i);
    }

    using callable_tag_type = _Configurable_rotate_left;
};

__RAZE_VX_NAMESPACE_END
