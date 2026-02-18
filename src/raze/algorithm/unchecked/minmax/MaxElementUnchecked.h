#pragma once 

#include <src/raze/algorithm/vectorized/minmax/MaxElementVectorized.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _UnwrappedInputIterator_>
__simd_nodiscard_inline_constexpr _UnwrappedInputIterator_ __max_element_unchecked(
	_UnwrappedInputIterator_ __first_unwrapped,
	_UnwrappedInputIterator_ __last_unwrapped) noexcept
{
	using _ValueType = type_traits::iterator_value_type<_UnwrappedInputIterator_>;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _ValueType>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{ 
			const auto __first_address	= std::to_address(__first_unwrapped);
			const auto __position		= __max_element_vectorized<_ValueType>(__first_address, std::to_address(__last_unwrapped));

			if constexpr (std::is_pointer_v<_UnwrappedInputIterator_>)
				return __position;
			else
				return __first_unwrapped + (__position - __first_address);
		}
	}

	if (__first_unwrapped == __last_unwrapped)
		return __last_unwrapped;

	auto __max = __first_unwrapped;

	for (; ++__first_unwrapped != __last_unwrapped; )
		if (*__first_unwrapped > *__max)
			__max = __first_unwrapped;

	return __max;
}

template <
	class _UnwrappedInputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _UnwrappedInputIterator_ __max_element_unchecked(
	_UnwrappedInputIterator_	__first_unwrapped,
	_UnwrappedInputIterator_	__last_unwrapped,
	_Predicate_					__predicate) noexcept
{
	using _ValueType = type_traits::iterator_value_type<_UnwrappedInputIterator_>;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _ValueType>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{ 
			const auto __first_address	= std::to_address(__first_unwrapped);
			const auto __position		= __max_element_vectorized<_ValueType>(__first_address, std::to_address(__last_unwrapped));

			if constexpr (std::is_pointer_v<_UnwrappedInputIterator_>)
				return __position;
			else
				return __first_unwrapped + (__position - __first_address);
		}
	}

	if (__first_unwrapped == __last_unwrapped)
		return __last_unwrapped;

	auto __maximum = __first_unwrapped;

	for (; ++__first_unwrapped != __last_unwrapped; )
		if (__predicate(__maximum, __first_unwrapped))
			__maximum = __first_unwrapped;

	return __maximum;
}


__RAZE_ALGORITHM_NAMESPACE_END
