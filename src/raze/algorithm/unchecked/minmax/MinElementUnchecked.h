#pragma once 

#include <src/raze/algorithm/vectorized/minmax/MinElementVectorized.h>
#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedForwardIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _UnwrappedForwardIterator_ __min_element_unchecked(
	_UnwrappedForwardIterator_	__first_unwrapped,
	_UnwrappedForwardIterator_	__last_unwrapped,
	_Predicate_					__predicate) noexcept(type_traits::is_nothrow_invocable_v<_Predicate_,
		type_traits::iterator_value_type<_UnwrappedForwardIterator_>,
		type_traits::iterator_value_type<_UnwrappedForwardIterator_>>)
{
	using _ValueType = type_traits::iterator_value_type<_UnwrappedForwardIterator_>;

	if (__first_unwrapped == __last_unwrapped)
		return __last_unwrapped;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedForwardIterator_, _ValueType>
		&& type_traits::is_any_of_v<_Predicate_, std::less<>, type_traits::less<>>)
	{
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __first_address = std::to_address(__first_unwrapped);
			const auto __position = __min_element_vectorized<_ValueType>(__first_address, std::to_address(__last_unwrapped));

			return __first_unwrapped + (__position - __first_address);
		}
	}

	auto __minimum = __first_unwrapped;

	for (; ++__first_unwrapped != __last_unwrapped; )
		if (__predicate(*__first_unwrapped, *__minimum))
			__minimum = __first_unwrapped;

	return __minimum;
}

template <class _UnwrappedForwardIterator_>
__simd_nodiscard_inline_constexpr _UnwrappedForwardIterator_ __min_element_unchecked(
	_UnwrappedForwardIterator_ __first_unwrapped,
	_UnwrappedForwardIterator_ __last_unwrapped) noexcept(type_traits::is_nothrow_invocable_v<type_traits::less<>,
		type_traits::iterator_value_type<_UnwrappedForwardIterator_>,
		type_traits::iterator_value_type<_UnwrappedForwardIterator_>>)
{
	return __min_element_unchecked(__first_unwrapped, __last_unwrapped, type_traits::less<>{});
}


__RAZE_ALGORITHM_NAMESPACE_END
