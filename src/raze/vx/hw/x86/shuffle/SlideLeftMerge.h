#pragma once 

#include <src/raze/vx/hw/x86/shuffle/SlideRightMerge.h>

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(off)
#endif // defined(raze_cpp_msvc)

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V, sizetype Slide>
raze_always_inline V slide_left_merge_(const V& x, const V& y, std::integral_constant<sizetype, Slide> slide) noexcept {
    return slide_right_merge_(y, x, std::integral_constant<sizetype, V::size() - Slide>{});
}

template <simd_type V>
raze_always_inline V slide_left_merge_(const V& x, const V& y, i32 sh) noexcept {
    return slide_right_merge_(y, x, V::size() - sh);
}

__RAZE_VX_NAMESPACE_END

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(on)
#endif // defined(raze_cpp_msvc)