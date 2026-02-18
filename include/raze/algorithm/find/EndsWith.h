#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>

#include <src/raze/algorithm/vectorized/find/SearchVectorized.h>
#include <src/raze/type_traits/CanMemcmpElements.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/type_traits/OperatorWrappers.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_>
raze_nodiscard raze_always_inline raze_constexpr_cxx20 bool ends_with(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2,
	_Predicate_				__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	using _Value_ = type_traits::iterator_value_type<_FirstForwardIterator_>;

	using _FirstForwardIteratorUnwrappedType_	= __unwrapped_iterator_type<_FirstForwardIterator_>;
	using _SecondForwardIteratorUnwrappedType_	= __unwrapped_iterator_type<_SecondForwardIterator_>;

	__verify_range(__first1, __last1);
	__verify_range(__first2, __last2);

	auto __first1_unwrapped	= __unwrap_iterator(__first1);
	auto __first2_unwrapped	= __unwrap_iterator(__first2);

	auto __last1_unwrapped	= __unwrap_iterator(__last1);
	auto __last2_unwrapped	= __unwrap_iterator(__last2);

	if constexpr (
		type_traits::is_iterator_random_ranges_v<_FirstForwardIteratorUnwrappedType_> &&
		type_traits::is_iterator_random_ranges_v<_SecondForwardIteratorUnwrappedType_>
	)
	{
		const auto __first_range_length		= __iterators_difference(__first1_unwrapped, __last1_unwrapped);
		const auto __second_range_length	= __iterators_difference(__first2_unwrapped, __last2_unwrapped);

		if (__first_range_length < __second_range_length)
			return false;

		if (__second_range_length == 1)
			return (__predicate(*--__last1_unwrapped, *__first2_unwrapped));

		__first1_unwrapped += (__first_range_length - __second_range_length);

		for (auto __current = sizetype(0); __current < __second_range_length; ++__current, ++__first1_unwrapped, ++__first2_unwrapped)
			if (__predicate(*__first1_unwrapped, *__first2_unwrapped) == false)
				return false;
		
		return true;
	}
	else if constexpr (
		type_traits::is_iterator_forward_ranges_v<_FirstForwardIteratorUnwrappedType_> &&
		type_traits::is_iterator_forward_ranges_v<_SecondForwardIteratorUnwrappedType_>
	)
	{
		const auto __second_range_length = distance(__first2_unwrapped, __last2_unwrapped);

		if (__second_range_length == 0)
			return true;

		const auto __first_range_length = distance(__first1_unwrapped, __last1_unwrapped);

		if (__first_range_length < __second_range_length)
			return false;

		std::advance(__first1_unwrapped, __first_range_length - __second_range_length);

		for (; __first2_unwrapped != __last2_unwrapped; ++__first1_unwrapped, ++__first2_unwrapped)
			if (!__predicate(*__first1_unwrapped, *__first2_unwrapped))
				return false;

		return true;
	}
	else if constexpr (
		type_traits::is_iterator_bidirectional_ranges_v<_FirstForwardIteratorUnwrappedType_> &&
		type_traits::is_iterator_bidirectional_ranges_v<_SecondForwardIteratorUnwrappedType_>
	)
	{
		while (__last2_unwrapped != __first2_unwrapped) {
			if (__last1_unwrapped == __first1_unwrapped)
				return false;
			
			--__last1_unwrapped;
			--__last2_unwrapped;

			if (!__predicate(*__last1_unwrapped, *__last2_unwrapped))
				return false;
		}

		return true;
	}
}

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
raze_nodiscard raze_always_inline raze_constexpr_cxx20 bool ends_with(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::ends_with(__first1, __last1, __first2, __last2, type_traits::equal_to<>{});
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline raze_constexpr_cxx20 bool ends_with(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2,
	_Predicate_				__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::ends_with(__first1, __last1, __first2, __last2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline raze_constexpr_cxx20 bool ends_with(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::ends_with(__first1, __last1, __first2, __last2);
}

__RAZE_ALGORITHM_NAMESPACE_END
