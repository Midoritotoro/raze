#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/x86/arithmetic/Fms.h>
#include <src/raze/vx/expression/IsWhere.h>
#include <src/raze/vx/expression/IsWhereZero.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ __fms(
	_Type_ __x, _Type_ __y, _Type_ __z) noexcept
		requires(std::is_arithmetic_v<_Type_>)
{
	return __x * __y - __z;
}

template <class _Simd_>
raze_nodiscard raze_always_inline _Simd_ __fms(
	const _Simd_& __x,
	const _Simd_& __y,
	const _Simd_& __z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_>)
{
	using _RawType = std::remove_cvref_t<_Simd_>;
	return vx::_Fms<_RawType::__isa, _RawType::__width, typename _RawType::value_type>()(
		vx::__data(__x), vx::__data(__y), vx::__data(__z));
}

template <
	class _Simd_,
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ __fms(
	const _Simd_&				__x,
	const _Simd_&				__y,
	const _WhereExpression_&	__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> &&
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	using _RawSimdType = std::remove_cvref_t<_Simd_>;

	if constexpr (vx::__is_where_v<_WhereExpression_>)
		return vx::_Blend<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::_Fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
				vx::__data(__x), vx::__data(__y), vx::__data(__z.data())), vx::__data(__z.source()), vx::__data(__z.mask()));
	else
		return vx::_Maskz_fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::__data(__x), vx::__data(__y), vx::__data(__z.data()), vx::__data(__z.mask()));
}

template <
	class _Simd_,
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ __fms(
	const _Simd_&				__x,
	const _WhereExpression_&	__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> &&
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	using _RawSimdType = std::remove_cvref_t<_Simd_>;

	if constexpr (vx::__is_where_v<_WhereExpression_>)
		return vx::_Blend<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::_Fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
				vx::__data(__x), vx::__data(__y.data()), vx::__data(__z)), vx::__data(__y.source()), vx::__data(__y.mask()));
	else
		return vx::_Maskz_fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::__data(__x), vx::__data(__y.data()), vx::__data(__z), vx::__data(__y.mask()));
}

template <
	class _Simd_,
	class _WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ __fms(
	const _WhereExpression_&	__x,
	const _Simd_&				__y,
	const _Simd_&				__z) noexcept
		requires(vx::__is_valid_simd_v<_Simd_> &&
			(vx::__is_where_v<_WhereExpression_> || vx::__is_where_zero_v<_WhereExpression_>))
{
	using _RawSimdType = std::remove_cvref_t<_Simd_>;

	if constexpr (vx::__is_where_v<_WhereExpression_>)
		return vx::_Blend<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::_Fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
				vx::__data(__x.data()), vx::__data(__y), vx::__data(__z)), vx::__data(__x.source()), vx::__data(__x.mask()));
	else
		return vx::_Maskz_fms<_RawSimdType::__isa, _RawSimdType::__width, typename _RawSimdType::value_type>()(
			vx::__data(__x.data()), vx::__data(__y), vx::__data(__z), vx::__data(__x.mask()));
}

__RAZE_MATH_NAMESPACE_END
