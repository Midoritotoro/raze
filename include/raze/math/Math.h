#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>
#include <src/raze/math/Fnma.h>
#include <src/raze/math/Fnms.h>
#include <src/raze/math/Sin.h>
#include <src/raze/math/Cos.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <typename _Type_>
__simd_nodiscard_inline_constexpr _Type_ abs(_Type_ __value) noexcept
	requires(std::is_arithmetic_v<_Type_>)
{
	if constexpr (std::is_unsigned_v<_Type_>)
		return __value;
	else
		return (__value < 0) ? -__value : __value;
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ fma(
	_Type_ __x, _Type_ __y, _Type_ __z) noexcept
		requires(std::is_arithmetic_v<_Type_>)
{
	return __fma(__x, __y, __z);
}

template <class _Simd_>
raze_nodiscard raze_always_inline _Simd_ fma(
	const _Simd_& __x, 
	const _Simd_& __y, 
	const _Simd_& __z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_>)
{
	return __fma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fma(
	const _Simd_&				__x,
	const _Simd_&				__y,
	const _WhereExpression_&	__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fma(
	const _Simd_&				__x,
	const _WhereExpression_&	__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fma(
	const _WhereExpression_&	__x,
	const _Simd_&				__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fma(__x, __y, __z);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ fms(
	_Type_ __x, _Type_ __y, _Type_ __z) noexcept
		requires(std::is_arithmetic_v<_Type_>)
{
	return __fms(__x, __y, __z);
}


template <class _Simd_>
raze_nodiscard raze_always_inline _Simd_ fms(
	const _Simd_& __x, 
	const _Simd_& __y, 
	const _Simd_& __z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_>)
{
	return __fms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fms(
	const _Simd_&				__x,
	const _Simd_&				__y,
	const _WhereExpression_&	__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fms(
	const _Simd_&				__x,
	const _WhereExpression_&	__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fms(
	const _WhereExpression_&	__x,
	const _Simd_&				__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fms(__x, __y, __z);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ fnms(
	_Type_ __x, _Type_ __y, _Type_ __z) noexcept
		requires(std::is_arithmetic_v<_Type_>)
{
	return __fnms(__x, __y, __z);
}

template <class _Simd_>
raze_nodiscard raze_always_inline _Simd_ fnms(
	const _Simd_& __x, 
	const _Simd_& __y, 
	const _Simd_& __z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_>)
{
	return __fnms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnms(
	const _Simd_&				__x,
	const _Simd_&				__y,
	const _WhereExpression_&	__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnms(
	const _Simd_&				__x,
	const _WhereExpression_&	__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnms(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnms(
	const _WhereExpression_&	__x,
	const _Simd_&				__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnms(__x, __y, __z);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ fnma(
	_Type_ __x, _Type_ __y, _Type_ __z) noexcept
		requires(std::is_arithmetic_v<_Type_>)
{
	return __fnma(__x, __y, __z);
}

template <class _Simd_>
raze_nodiscard raze_always_inline _Simd_ fnma(
	const _Simd_& __x, 
	const _Simd_& __y, 
	const _Simd_& __z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_>)
{
	return __fnma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnma(
	const _Simd_&				__x,
	const _Simd_&				__y,
	const _WhereExpression_&	__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnma(
	const _Simd_&				__x,
	const _WhereExpression_&	__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnma(__x, __y, __z);
}

template <
	class _Simd_, 
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ fnma(
	const _WhereExpression_&	__x,
	const _Simd_&				__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> && 
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	return __fnma(__x, __y, __z);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ sin(_Type_ __x) noexcept {
	return __sin(__x);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ cos(_Type_ __x) noexcept {
	return __cos(__x);
}

__RAZE_MATH_NAMESPACE_END
