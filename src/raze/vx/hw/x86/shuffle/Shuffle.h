#pragma once 

#include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#include <src/raze/vx/hw/x86/shuffle/RotateLeft.h>
#include <src/raze/vx/hw/x86/shuffle/RotateRight.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/shuffle/Splat.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Pattern> 
raze_always_inline pattern_vector_t<Pattern> shuffle_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	if constexpr (is_reverse(p)) {
		return reverse_(x, p);
	}
	else if constexpr (is_splat(p)) {
		return splat_(x, p);
	}
	else {
		return generic_shuffle_(x, p);
	}
}

template <simd_type V, index_simd_type Index>
raze_always_inline V shuffle_(const V& x, const Index& idx) noexcept
	requires (index_type_for<Index, V>)
{
	return generic_shuffle_(x, idx);
}

__RAZE_VX_NAMESPACE_END

