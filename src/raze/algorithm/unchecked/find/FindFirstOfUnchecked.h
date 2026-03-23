#pragma once 

#include <src/raze/algorithm/vectorized/find/FindFirstOfVectorized.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedFirstForwardIterator_,
	class _UnwrappedSecondForwardIterator_,
	class _Predicate_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _UnwrappedFirstForwardIterator_ __find_first_of_unchecked(
	_UnwrappedFirstForwardIterator_		__first1_unwrapped,
	_UnwrappedFirstForwardIterator_		__last1_unwrapped,
	_UnwrappedSecondForwardIterator_	__first2_unwrapped,
	_UnwrappedSecondForwardIterator_	__last2_unwrapped,
	_Predicate_							__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_UnwrappedFirstForwardIterator_>,
			type_traits::iterator_value_type<_UnwrappedSecondForwardIterator_>>)
{
	using _ValueType = type_traits::iterator_value_type<_UnwrappedFirstForwardIterator_>;

	if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
		_UnwrappedFirstForwardIterator_, _UnwrappedSecondForwardIterator_, _Predicate_>)
	{
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __first1_address = std::to_address(__first1_unwrapped);

			const auto __result = __find_first_of_vectorized<_ValueType>(
				__first1_address, std::to_address(__last1_unwrapped),
				std::to_address(__first2_unwrapped), std::to_address(__last2_unwrapped));

			return __first1_unwrapped + (reinterpret_cast<const _ValueType*>(__result) - __first1_address);
		}
	}

	for (; __first1_unwrapped != __last1_unwrapped; ++__first1_unwrapped)
		for (auto __mid2_unwrapped = __first2_unwrapped; __mid2_unwrapped != __last2_unwrapped; ++__mid2_unwrapped)
			if (__predicate(*__first1_unwrapped, *__mid2_unwrapped)) 
				return __first1_unwrapped;

	return __last1_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
