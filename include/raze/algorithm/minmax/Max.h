#pragma once 

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__simd_nodiscard_inline constexpr _Type_ max(
	const _Type_& __left,
	const _Type_& __right) noexcept
{
	return (__left > __right) ? __left : __right;
}

template <
	class _Type_,
	class _Predicate_>
__simd_nodiscard_inline constexpr _Type_ max(
	const _Type_&	__left,
	const _Type_&	__right,
	_Predicate_		__predicate) noexcept
{
	return __predicate(__left, __right) ? __right : __left;
}

__RAZE_ALGORITHM_NAMESPACE_END
