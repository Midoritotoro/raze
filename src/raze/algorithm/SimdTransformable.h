#pragma once 

#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/OperatorWrappers.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _Function_,
	class _Simd_>
struct is_simd_unary_transformable:
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<std::negate<typename _Simd_::value_type>, _Simd_>:
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<std::bit_not<typename _Simd_::value_type>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<std::negate<void>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<std::bit_not<void>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<type_traits::negate<typename _Simd_::value_type>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<type_traits::bit_not<typename _Simd_::value_type>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<type_traits::negate<void>, _Simd_> :
	std::false_type
{};

template <class _Simd_>
struct is_simd_unary_transformable<type_traits::bit_not<void>, _Simd_> :
	std::false_type
{};

template <
	class _Function_,
	class _Simd_>
constexpr auto is_simd_unary_transformable_v = is_simd_unary_transformable<_Simd_, _Function_>::value;


template <
	class _Function_,
	class _Simd_>
struct is_simd_binary_transformable:
	std::false_type
{};


template <class _Simd_>
struct is_simd_binary_transformable<std::bit_xor<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::bit_or<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::bit_and<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::plus<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::minus<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::multiplies<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::divides<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::bit_xor<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::bit_or<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::bit_and<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::plus<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::minus<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::multiplies<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<std::divides<void>, _Simd_> :
	std::true_type
{};



template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_xor<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_or<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_and<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::plus<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::minus<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::multiplies<typename _Simd_::value_type>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::divides<typename _Simd_::value_type>, _Simd_>: 
	std::true_type 
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_xor<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_or<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::bit_and<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::plus<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::minus<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::multiplies<void>, _Simd_> :
	std::true_type
{};

template <class _Simd_>
struct is_simd_binary_transformable<type_traits::divides<void>, _Simd_> :
	std::true_type
{};

template <
	class _Function_,
	class _Simd_>
constexpr auto is_simd_binary_transformable_v = is_simd_binary_transformable<_Function_, _Simd_>::value;

__RAZE_ALGORITHM_NAMESPACE_END
