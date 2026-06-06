#pragma once 

#include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#include <src/raze/vx/hw/x86/shuffle/RotateLeft.h>
#include <src/raze/vx/hw/x86/shuffle/RotateRight.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/shuffle/Splat.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __shuffle(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;
	
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

__RAZE_VX_NAMESPACE_END

