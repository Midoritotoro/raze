#pragma once 

#include <src/raze/vx/hw/x86/shuffle/SlideRightMerge.h>

#pragma strict_gs_check(off)

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_, sizetype _Slide_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_merge(
    const _Simd_& __x, const _Simd_& __y, std::integral_constant<sizetype, _Slide_> __slide) noexcept
{
    return __slide_right_merge(__y, __x, std::integral_constant<sizetype, _Simd_::size() - _Slide_>{});
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_merge(
    const _Simd_& __x, const _Simd_& __y, i32 __sh) noexcept 
{
    return __slide_right_merge(__y, __x, _Simd_::size() - __sh);
}

__RAZE_VX_NAMESPACE_END

#pragma strict_gs_check(on)