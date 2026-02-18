#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/remove/RemoveVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/datapar/IsComparable.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedInputIterator_,
	class _Type_ = type_traits::iterator_value_type<_UnwrappedInputIterator_>>
__simd_nodiscard_inline_constexpr _UnwrappedInputIterator_ __remove_unchecked(
	_UnwrappedInputIterator_							__first_unwrapped,
	_UnwrappedInputIterator_							__last_unwrapped,
	const typename std::type_identity<_Type_>::type&	__value) noexcept
{
	using _DifferenceType = type_traits::iterator_difference_type<_UnwrappedInputIterator_>;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _Type_>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __first_address = std::to_address(__first_unwrapped);
			const auto __position = __remove_vectorized<type_traits::iterator_value_type<_UnwrappedInputIterator_>>(
				__first_address, std::to_address(__last_unwrapped), __value);

			if constexpr (std::is_pointer_v<_UnwrappedInputIterator_>)
				return __position;
			else
				return __first_unwrapped + (__position - __first_address);
		}
	}

	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		if (*__first_unwrapped == __value)
			break;

	for (auto __current = __first_unwrapped; ++__current != __last_unwrapped;)
		if (*__current != __value)
			*__first_unwrapped++ = *__current;

	return __first_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
