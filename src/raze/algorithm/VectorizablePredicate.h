#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Iterator_>
using __simd_value_t = vx::simd<std::iter_value_t<_Iterator_>, vx::default_abi>;

template <class _Iterator_>
using __simd_mask_value_t = vx::simd_mask<std::iter_value_t<_Iterator_>, vx::default_abi>;

template <class _Function_>
concept __has_function_unwrapped_type = requires {
    typename _Function_::function_unwrapped_type;
};

template <class _Function_>
struct __function_unwrapped_impl {
    using type = _Function_;
};

template <__has_function_unwrapped_type _Function_>
struct __function_unwrapped_impl<_Function_> {
    using type = typename _Function_::function_unwrapped_type;
};

template <class _Function_>
using __function_unwrapped = typename __function_unwrapped_impl<_Function_>::type;

template <class _Type_>
concept vectorizable_value_type = type_traits::__is_vector_type_supported_v<std::decay_t<_Type_>>;

template <class _Predicate_, class _Iterator_>
concept always_scalar = !std::invocable<std::remove_cvref_t<_Predicate_>, __simd_value_t<_Iterator_>> ||
	requires { { std::invoke(std::declval<std::remove_cvref_t<_Predicate_>>(), std::declval<__simd_value_t<_Iterator_>>()) } -> std::convertible_to<bool>; };

template <class _Predicate_, class _Iterator_>
concept vectorizable_unary_predicate = !always_scalar<_Predicate_, _Iterator_> && vectorizable_value_type<std::iter_value_t<_Iterator_>> && requires {
	{ std::invoke(std::declval< __function_unwrapped<std::remove_cvref_t<_Predicate_>>>(), std::declval<__simd_value_t<_Iterator_>>()) }
		-> std::convertible_to<__simd_mask_value_t<_Iterator_>>;
};

template <class _Predicate_, class _Iterator_>
concept vectorizable_binary_predicate = !always_scalar<_Predicate_, _Iterator_> && vectorizable_value_type<std::iter_value_t<_Iterator_>> && requires {
	{ std::invoke(std::declval<__function_unwrapped<std::remove_cvref_t<_Predicate_>>>(), std::declval<__simd_value_t<_Iterator_>>(),
		std::declval<__simd_value_t<_Iterator_>>()) } -> std::convertible_to<__simd_mask_value_t<_Iterator_>>;
};

template <class _Projection_, class _Iterator_>
concept vectorizable_projection = requires {
	{ std::invoke(std::declval<_Projection_>(), std::declval<__simd_value_t<_Iterator_>>()) }
		-> std::convertible_to<__simd_value_t<_Iterator_>>;
};


__RAZE_ALGORITHM_NAMESPACE_END
