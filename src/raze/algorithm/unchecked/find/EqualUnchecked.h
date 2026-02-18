#pragma once

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/find/EqualVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/type_traits/CanMemcmpElements.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstUnwrappedIterator_,
	class _SecondUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr bool __equal_unchecked(
	_FirstUnwrappedIterator_	__first1_unwrapped,
	_FirstUnwrappedIterator_	__last1_unwrapped,
	_SecondUnwrappedIterator_	__first2_unwrapped,
	_Predicate_					__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, 
			type_traits::iterator_value_type<_FirstUnwrappedIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedIterator_>>)
{
	using _DifferenceType = type_traits::iterator_difference_type<_FirstUnwrappedIterator_>;

	constexpr auto __is_random = type_traits::is_iterator_random_ranges_v<_FirstUnwrappedIterator_> &&
		type_traits::is_iterator_random_ranges_v<_SecondUnwrappedIterator_>;

	if constexpr (__is_random) {
		const auto __size = __iterators_difference(__first1_unwrapped, __last1_unwrapped);

		if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
			_FirstUnwrappedIterator_, _SecondUnwrappedIterator_, _Predicate_>)
		{
#if raze_has_cxx20
			if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
			{
				return __equal_vectorized<type_traits::iterator_value_type<_FirstUnwrappedIterator_>>(
					std::to_address(__first1_unwrapped), std::to_address(__first2_unwrapped), __size);
			}
		}
		else {
			for (auto __current = _DifferenceType(0); __current < __size; ++__current)
				if (__predicate(*__first1_unwrapped++, *__first2_unwrapped++) == false)
					return false;

			return true;
		}
	}

	for (; __first1_unwrapped != __last1_unwrapped; ++__first1_unwrapped, ++__first2_unwrapped)
		if (__predicate(*__first1_unwrapped, *__first2_unwrapped) == false)
			return false;
	
	return true;
}

template <
	class _FirstUnwrappedIterator_,
	class _SecondUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr bool __equal_unchecked(
	_FirstUnwrappedIterator_	__first1_unwrapped,
	_FirstUnwrappedIterator_	__last1_Unwrapped,
	_SecondUnwrappedIterator_	__first2_Unwrapped,
	_SecondUnwrappedIterator_	__last2_Unwrapped,
	_Predicate_					__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedIterator_>,
			type_traits::iterator_value_type<_FirstUnwrappedIterator_>>)
{
	using _DifferenceType = type_traits::iterator_difference_type<_FirstUnwrappedIterator_>;

	constexpr auto __is_random = type_traits::is_iterator_random_ranges_v<_FirstUnwrappedIterator_> &&
		type_traits::is_iterator_random_ranges_v<_SecondUnwrappedIterator_>;

	if constexpr (__is_random) {
		const auto __size = __iterators_difference(__first1_unwrapped, __last1_Unwrapped);

		if (__size != __iterators_difference(__first2_Unwrapped, __last2_Unwrapped))
			return false;

		if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
			_FirstUnwrappedIterator_, _SecondUnwrappedIterator_, _Predicate_>)
		{
#if raze_has_cxx20
			if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
			{
				return __equal_vectorized<type_traits::iterator_value_type<_FirstUnwrappedIterator_>>(
					std::to_address(__first1_unwrapped), std::to_address(__first2_Unwrapped), __size);
			}
		}
		else {
			for (auto __current = _DifferenceType(0); __current < __size; ++__current)
				if (__predicate(*__first1_unwrapped++, *__first2_Unwrapped++) == false)
					return false;

			return true;
		}
	}

	for (; __first1_unwrapped != __last1_Unwrapped; ++__first1_unwrapped, ++__first2_Unwrapped)
		if (__predicate(*__first1_unwrapped, *__first2_Unwrapped) == false)
			return false;
	
	return true;
}


__RAZE_ALGORITHM_NAMESPACE_END
