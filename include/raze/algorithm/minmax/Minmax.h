#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__simd_nodiscard_inline std::pair<_Type_, _Type_> minmax(
	const _Type_& __left,
	const _Type_& __right) noexcept
{
	return (__left < __right) 
		? std::pair<_Type_, _Type_>{ __left, __right } 
		: std::pair<_Type_, _Type_>{ __right, __left };
}

template <
	class _Type_,
	class _Predicate_>
__simd_nodiscard_inline std::pair<_Type_, _Type_> minmax(
	const _Type_&	__left,
	const _Type_&	__right,
	_Predicate_		__predicate) noexcept
{
	return __predicate(__right, __left) 
		? std::pair<_Type_, _Type_>{ __right, __left } 
		: std::pair<_Type_, _Type_>{ __left, __right };
}

__RAZE_ALGORITHM_NAMESPACE_END
