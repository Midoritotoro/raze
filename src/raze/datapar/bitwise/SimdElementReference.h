#pragma once 

#include <src/raze/datapar/SimdIntegralTypesCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Simd_> 
class _Simd_element_reference {
public:
	using value_type = typename _Simd_::value_type;

	_Simd_element_reference(_Simd_& __simd, int32 __i) noexcept :
		_reference(__simd),
		_index(__i)
	{}

	raze_always_inline operator value_type() const noexcept {
		return _reference[_index];
	}

	raze_always_inline _Simd_element_reference operator=(value_type __value) noexcept && {
		_reference.insert(_index, __value);
		return { _reference, _index };
	}

	raze_always_inline _Simd_element_reference operator=(value_type __value) noexcept&& {
		_reference.insert(_index, __value);
		return { _reference, _index };
	}
private:
	_Simd_& _reference;
	int32 _index = 0;
};

__RAZE_DATAPAR_NAMESPACE_END
