#pragma once 

#include <src/raze/datapar/bitwise/MaskImplementation.h>
#include <src/raze/datapar/bitwise/BitReference.h>
#include <src/raze/datapar/bitwise/SimdBoolReference.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_SimdWidth_>
using __mask_element_reference = std::conditional_t<std::is_integral_v<typename _Mask_implementation<_ISA_, _Type_, _SimdWidth_>::mask_type>,
	_Bit_reference<typename _Mask_implementation<_ISA_, _Type_, _SimdWidth_>::mask_type>,
	_Simd_bool_reference<typename _Mask_implementation<_ISA_, _Type_, _SimdWidth_>::mask_type>>;

__RAZE_DATAPAR_NAMESPACE_END
