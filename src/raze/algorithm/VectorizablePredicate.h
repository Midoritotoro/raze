#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Predicate_, class _Iterator_>
concept vectorizable_unary_predicate = requires {
	{ std::invoke(std::declval<_Predicate_>(), std::declval<vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>>()) } 
		-> std::convertible_to<vx::simd_mask<std::iter_value_t<_Iterator_>, vx::default_abi>>;
};

template <class _Predicate_, class _Iterator_>
concept vectorizable_binary_predicate = requires {
	{ std::invoke(std::declval<_Predicate_>(), std::declval<vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>>(),
		std::declval<vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>>()) } 
			-> std::convertible_to<vx::simd_mask<std::iter_value_t<_Iterator_>, vx::default_abi>>;
};

template <class _Projection_, class _Iterator_>
concept vectorizable_projection = requires {
	{ std::invoke(std::declval<_Projection_>(), std::declval<vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>>()) } 
		-> std::convertible_to<vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>>;
};


__RAZE_ALGORITHM_NAMESPACE_END
