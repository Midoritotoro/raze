#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
constexpr raze_always_inline _Type_ min(
	const _Type_& __left,
	const _Type_& __right) noexcept
{
	return (__left < __right) ? __left : __right;
}

template <
	class _Type_,
	class _Predicate_>
constexpr raze_always_inline _Type_ min(
	const _Type_&	__left,
	const _Type_&	__right,
	_Predicate_		__predicate) noexcept
{
	return __predicate(__right, __left) ? __right : __left;
}

__RAZE_ALGORITHM_NAMESPACE_END
