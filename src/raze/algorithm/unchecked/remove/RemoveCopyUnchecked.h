#pragma once 

#include <src/raze/algorithm/vectorized/remove/RemoveCopyVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/datapar/IsComparable.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedInputIterator_,
	class _UnwrappedOutputIterator_,
	class _Type_ = type_traits::iterator_value_type<_UnwrappedInputIterator_>>
__simd_nodiscard_inline_constexpr _UnwrappedOutputIterator_ __remove_copy_unchecked(
	_UnwrappedInputIterator_							__first_unwrapped,
	_UnwrappedInputIterator_							__last_unwrapped,
	_UnwrappedOutputIterator_							__destination_unwrapped,
	const typename std::type_identity<_Type_>::type&	__value) noexcept
{
	constexpr auto __is_vectorizable = type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _Type_> &&
		type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedOutputIterator_, _Type_> &&
		type_traits::__copy_iterator_category<_UnwrappedInputIterator_, _UnwrappedOutputIterator_>::BitcopyAssignable;

	using _DifferenceType = type_traits::iterator_difference_type<_UnwrappedOutputIterator_>;

	if constexpr (__is_vectorizable) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __destination_address = std::to_address(__destination_unwrapped);
			auto __position					 = __remove_copy_vectorized<_Type_>(std::to_address(__first_unwrapped),
				std::to_address(__last_unwrapped), __destination_address, __value);

			if constexpr (std::is_pointer_v<_UnwrappedOutputIterator_>)
				return __position;
			else
				return __destination_unwrapped + static_cast<_DifferenceType>(__position - __destination_address);
		}
	}

	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		if (*__first_unwrapped != __value)
			*__destination_unwrapped++ = *__first_unwrapped;
	
	return __destination_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
