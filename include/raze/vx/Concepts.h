#pragma once 

#include <raze/vx/Abi.h>
#include <raze/vx/SimdBitmask.h>
#include <raze/vx/BasicSimdMask.h>
#include <src/raze/vx/expression/IsConstWhere.h>
#include <src/raze/vx/expression/IsWhere.h>
#include <src/raze/vx/expression/IsConstWhereZero.h>
#include <src/raze/vx/expression/IsWhereZero.h>
#include <src/raze/vx/expression/IsWhereZeroMemory.h>
#include <src/raze/vx/expression/IsWhereMemory.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _WhereExpression_>
concept const_where_expression_type = __is_const_where_v<_WhereExpression_> ||
	__is_const_where_zero_v<_WhereExpression_>;

template <class _WhereExpression_>
concept where_expression_type = __is_where_v<_WhereExpression_> ||
	__is_where_zero_v<_WhereExpression_>;

template <class _WhereExpression_>
concept where_memory_expression_type = __is_where_memory_v<_WhereExpression_> ||
	__is_where_zero_memory_v<_WhereExpression_>;

template <class _WhereExpression_>
concept any_where_expression_type = where_memory_expression_type<_WhereExpression_> ||
	where_expression_type<_WhereExpression_> || const_where_expression_type<_WhereExpression_>;

template <class _WhereExpression_>
concept non_memory_where_expression_type = any_where_expression_type<_WhereExpression_> &&
	!where_memory_expression_type<_WhereExpression_>;

template <class _Simd_>
concept simd_type = __is_valid_simd_v<_Simd_>;

template <class _SimdMask_>
concept simd_mask_type = __is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>;

template <class _AlignmentPolicy_>
concept alignment_policy_type = requires {
	{ std::remove_cvref_t<_AlignmentPolicy_>::__alignment } -> std::convertible_to<bool>;
};

__RAZE_VX_NAMESPACE_END
