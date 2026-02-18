#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/find/MismatchVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/type_traits/CanMemcmpElements.h>
#include <src/raze/type_traits/FunctionPass.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstUnwrappedIterator_,
	class _SecondUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr std::pair<_FirstUnwrappedIterator_, _SecondUnwrappedIterator_> __mismatch_unchecked(
	_FirstUnwrappedIterator_		__first1_unwrapped,
	_FirstUnwrappedIterator_		__last1_unwrapped,
	_SecondUnwrappedIterator_		__first2_unwrapped,
	_Predicate_						__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedIterator_>>)
{
	if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
		_FirstUnwrappedIterator_, _SecondUnwrappedIterator_, _Predicate_>)
	{
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __position = __mismatch_vectorized<type_traits::iterator_value_type<_FirstUnwrappedIterator_>>(
				std::to_address(__first1_unwrapped), std::to_address(__first2_unwrapped),
				__iterators_difference(__first1_unwrapped, __last1_unwrapped));

			__first1_unwrapped += __position;
			__first2_unwrapped += __position;

			return { __first1_unwrapped, __first2_unwrapped };
		}
	}

	while (__first1_unwrapped != __last1_unwrapped && __predicate(*__first1_unwrapped, *__first2_unwrapped)) {
		++__first1_unwrapped;
		++__first2_unwrapped;
	}

	return { __first1_unwrapped, __first2_unwrapped };
}

template <
	class _FirstUnwrappedIterator_,
	class _SecondUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr std::pair<_FirstUnwrappedIterator_, _SecondUnwrappedIterator_> __mismatch_unchecked(
	_FirstUnwrappedIterator_	__first1_unwrapped,
	_FirstUnwrappedIterator_	__last1_unwrapped,
	_SecondUnwrappedIterator_	__first2_unwrapped,
	_SecondUnwrappedIterator_	__last2_unwrapped,
	_Predicate_					__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedIterator_>>)
{
	if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
		_FirstUnwrappedIterator_, _SecondUnwrappedIterator_, _Predicate_>)
	{
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __size = (std::min)(
				__iterators_difference(__first1_unwrapped, __last1_unwrapped), 
				__iterators_difference(__first2_unwrapped, __last2_unwrapped));

			const auto __position = __mismatch_vectorized<type_traits::iterator_value_type<_FirstUnwrappedIterator_>>(
				std::to_address(__first1_unwrapped), std::to_address(__first2_unwrapped), __size);
				
			__first1_unwrapped += __position;
			__first2_unwrapped += __position;

			return { __first1_unwrapped, __first2_unwrapped };
		}
	}

	while (__first1_unwrapped != __last1_unwrapped && __predicate(*__first1_unwrapped, *__first2_unwrapped)) {
		++__first1_unwrapped;
		++__first2_unwrapped;
	}

	return { __first1_unwrapped, __first2_unwrapped };
}

__RAZE_ALGORITHM_NAMESPACE_END
