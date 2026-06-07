#pragma once 

#include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#include <src/raze/vx/hw/x86/shuffle/RotateLeft.h>
#include <src/raze/vx/hw/x86/shuffle/RotateRight.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/shuffle/Splat.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __shuffle(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	if constexpr (__is_reverse(__p)) {
		return __reverse(__x, __p);
	}
	else if constexpr (__is_splat(__p)) {
		return __splat(__x, __p);
	}
	/*else if constexpr (__is_slide_left(__p)) {

	}
	else if constexpr (__is_slide_right(__p)) {
		 
	}
	else if constexpr (__is_rotate_left(__p)) {

	}
	else if constexpr (__is_rotate_right(__p)) {

	}*/
	else {
		return __generic_shuffle(__x, __p);
	}
}

template <simd_type _Simd_, index_simd_type _Index_>
raze_always_inline _Simd_ __shuffle(const _Simd_& __x, const _Index_& __idx) noexcept
	requires (index_type_for<_Index_, _Simd_>)
{
	return __generic_shuffle(__x, __idx);
}

__RAZE_VX_NAMESPACE_END

