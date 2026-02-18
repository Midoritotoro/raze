#pragma once 

#include <src/raze/type_traits/IteratorCategory.h>
#include <src/raze/type_traits/OperatorWrappers.h>

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
	class _FirstElement_,
	class _SecondElement_,
	bool = sizeof(_FirstElement_) == sizeof(_SecondElement_)
		&& std::is_integral_v<_FirstElement_> || std::is_floating_point_v<_FirstElement_>
		&& std::is_integral_v<_SecondElement_> || std::is_floating_point_v<_SecondElement_>>
constexpr bool __can_memcmp_elements_v =
		std::is_same_v<_FirstElement_, bool>
	||	std::is_same_v<_SecondElement_, bool> 
	||	static_cast<_FirstElement_>(-1) == static_cast<_SecondElement_>(-1);

#if defined(__cpp_lib_byte)

template <>
inline constexpr bool __can_memcmp_elements_v<std::byte, std::byte, false> = true;

#endif // defined(__cpp_lib_byte)

template <
	class _FirstType_,
	class _SecondType_>
constexpr bool __can_memcmp_elements_v<_FirstType_*, _SecondType_*, false> =
	is_pointer_address_comparable_v<_FirstType_, _SecondType_>;


template <
	class _FirstElement_,
	class _SecondElement_>
constexpr bool __can_memcmp_elements_v<_FirstElement_, _SecondElement_, false> = false;

template <
	class _FirstElement_,
	class _SecondElement_,
	class _Function_>
constexpr bool __can_memcmp_elements_with_pred_v = false;

template <
	class _FirstElement_,
	class _SecondElement_, 
	class _ThirdElement_>
constexpr bool __can_memcmp_elements_with_pred_v<_FirstElement_, _SecondElement_, std::equal_to<_ThirdElement_>> =
	__copy_iterator_category<_FirstElement_*, _ThirdElement_*>::BitcopyConstructible &&
	__copy_iterator_category<_SecondElement_*, _ThirdElement_*>::BitcopyConstructible && 
	__can_memcmp_elements_v<std::remove_cv_t<_ThirdElement_>, std::remove_cv_t<_ThirdElement_>
>;

template <
	class _FirstElement_,
	class _SecondElement_>
constexpr bool __can_memcmp_elements_with_pred_v<_FirstElement_, _SecondElement_, type_traits::equal_to<>> =
	__can_memcmp_elements_v<_FirstElement_, _SecondElement_>;

#if raze_has_cxx20
template <
	class _FirstElement_,
	class _SecondElement_>
constexpr bool __can_memcmp_elements_with_pred_v<_FirstElement_, _SecondElement_, std::ranges::equal_to> =
	__can_memcmp_elements_v<_FirstElement_, _SecondElement_>;
#endif // raze_has_cxx20

template <
	class _FirstIterator_,
	class _SecondIterator_,
	class _Function_>
constexpr bool __equal_memcmp_is_safe_helper = 
	is_iterator_contiguous_v<_FirstIterator_> && is_iterator_contiguous_v<_SecondIterator_>
	&& !is_iterator_volatile_v<_FirstIterator_> && !is_iterator_volatile_v<_SecondIterator_>
	&& __can_memcmp_elements_with_pred_v<
		iterator_value_type<_FirstIterator_>,
		iterator_value_type<_SecondIterator_>, _Function_>;

template <
	class _FirstIterator_,
	class _SecondIterator_, 
	class _Function_>
constexpr bool __equal_memcmp_is_safe_v =
	__equal_memcmp_is_safe_helper<
		std::remove_const_t<_FirstIterator_>,
		std::remove_const_t<_SecondIterator_>,
		std::remove_const_t<_Function_>
	>;


template <
	class _FirstIterator_,
	class _SecondIterator_,
	class _Function_>
constexpr bool __is_vectorized_search_algorithm_safe_v = __equal_memcmp_is_safe_v<_FirstIterator_, _SecondIterator_, _Function_>;


__RAZE_TYPE_TRAITS_NAMESPACE_END
