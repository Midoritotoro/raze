#pragma once 

#include <src/raze/vx/bitwise/MaskImplementation.h>
#include <src/raze/vx/bitwise/BitReference.h>
#include <src/raze/vx/bitwise/SimdBoolReference.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _MaskType_>
raze_always_inline _Bit_reference<_MaskType_> __make_mask_element_reference(
	_MaskType_& __mask,
	int32		__index) noexcept
		requires(std::is_integral_v<_MaskType_>)
{
	return _Bit_reference(__mask, __index);
}

template <class _MaskType_> 
raze_always_inline _Simd_bool_reference<_MaskType_> __make_mask_element_reference(
	_MaskType_& __mask, 
	int32		__index) noexcept
		requires(__is_valid_simd_v<_MaskType_>)
{
	return _Simd_bool_reference(__mask, __index);
}

__RAZE_VX_NAMESPACE_END
