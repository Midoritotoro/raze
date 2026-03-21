#pragma once 

#include <src/raze/datapar/bitwise/MaskImplementation.h>
#include <src/raze/datapar/bitwise/BitReference.h>
#include <src/raze/datapar/bitwise/SimdBoolReference.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	class _Type_,
	class _Abi_>
using __mask_element_reference = std::conditional_t<std::is_integral_v<typename _Mask_implementation<_Type_, _Abi_>::mask_type>,
	_Bit_reference<typename _Mask_implementation<_Type_, _Abi_>::mask_type>,
	_Simd_bool_reference<typename _Mask_implementation<_Type_, _Abi_>::mask_type>>;

__RAZE_DATAPAR_NAMESPACE_END
