#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>

#include <src/raze/type_traits/CanMemcmpElements.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/type_traits/OperatorWrappers.h>

#include <src/raze/algorithm/vectorized/find/FindEndVectorized.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstUnwrappedBidirectionalIterator_,
	class _SecondUnwrappedBidirectionalIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedBidirectionalIterator_ __find_end_unchecked_bidirectional(
	_FirstUnwrappedBidirectionalIterator_	__first1_unwrapped,
	_FirstUnwrappedBidirectionalIterator_	__last1_unwrapped,
	_SecondUnwrappedBidirectionalIterator_	__first2_unwrapped,
	_SecondUnwrappedBidirectionalIterator_	__last2_unwrapped,
	_Predicate_								__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedBidirectionalIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedBidirectionalIterator_>>)
{
	for (auto __candidate_unwrapped = __last1_unwrapped;; --__candidate_unwrapped) {
		auto __next1_unwrapped = __candidate_unwrapped;
		auto __next2_unwrapped = __last2_unwrapped;

		for (;;) {
			if (__first2_unwrapped == __next2_unwrapped) {
				__seek_possibly_wrapped_iterator(__first1_unwrapped, __next1_unwrapped);
				return __first1_unwrapped;
			}

			if (__first1_unwrapped == __next1_unwrapped)
				return __last1_unwrapped;

			--__next1_unwrapped;
			--__next2_unwrapped;

			if (__predicate(*__next1_unwrapped, *__next2_unwrapped) == false)
				break;
		}
	}
}

template <
	class _FirstUnwrappedRandomAccessIterator_,
	class _SecondUnwrappedRandomAccessIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedRandomAccessIterator_ __find_end_unchecked_random_access(
	_FirstUnwrappedRandomAccessIterator_	__first1_unwrapped,
	_FirstUnwrappedRandomAccessIterator_	__last1_unwrapped,
	_SecondUnwrappedRandomAccessIterator_	__first2_unwrapped,
	_SecondUnwrappedRandomAccessIterator_	__last2_unwrapped,
	_Predicate_								__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedRandomAccessIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedRandomAccessIterator_>>)
{
	using _ValueType = type_traits::iterator_value_type<_FirstUnwrappedRandomAccessIterator_>;

	const auto __first_range_size	= __iterators_difference(__first1_unwrapped, __last1_unwrapped);
	const auto __second_range_size	= __iterators_difference(__first2_unwrapped, __last2_unwrapped);

	if (__first_range_size < __second_range_size || __second_range_size == 0)
		return __last1_unwrapped;

	if constexpr (type_traits::is_vectorized_search_algorithm_safe_v<
		_FirstUnwrappedRandomAccessIterator_, _SecondUnwrappedRandomAccessIterator_, _Predicate_>)
	{
		const auto __first1_address = std::to_address(__first1_unwrapped);
		const auto __first2_address = std::to_address(__first2_unwrapped);

		const auto __position = __find_end_vectorized<_ValueType>(
			__first1_address, __first_range_size, __first2_address, __second_range_size);

		if constexpr (std::is_pointer_v<_FirstUnwrappedRandomAccessIterator_>)
			__seek_possibly_wrapped_iterator(__first1_unwrapped, reinterpret_cast<const _ValueType*>(__position));
		else
			__seek_possibly_wrapped_iterator(__first1_unwrapped, __first1_unwrapped + (reinterpret_cast<const _ValueType*>(__position) - __first1_address));

		return __first1_unwrapped;
	}
	else { 
		return __find_end_unchecked_bidirectional(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, __predicate);
	}
}

template <
	class _FirstUnwrappedForwardIterator_,
	class _SecondUnwrappedForwardIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedForwardIterator_ __find_end_unchecked_forward(
	_FirstUnwrappedForwardIterator_		__first1_unwrapped,
	_FirstUnwrappedForwardIterator_		__last1_unwrapped,
	_SecondUnwrappedForwardIterator_	__first2_unwrapped,
	_SecondUnwrappedForwardIterator_	__last2_unwrapped,
	_Predicate_							__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedForwardIterator_>>)
{
	auto __result_unwrapped = __last1_unwrapped;

	for (;;) {
		auto __next1_unwrapped = __first1_unwrapped;
		auto __next2_unwrapped = __first2_unwrapped;

		for (;;) {
			const auto __needle_end = (__next2_unwrapped == __last2_unwrapped);

			if (__needle_end)
				__result_unwrapped = __first1_unwrapped;

			if (__next1_unwrapped == __last1_unwrapped) {
				__seek_possibly_wrapped_iterator(__first1_unwrapped, __result_unwrapped);
				return __first1_unwrapped;
			}

			if (__needle_end || __predicate(*__next1_unwrapped, *__next2_unwrapped) == false)
				++__next1_unwrapped;

			++__next2_unwrapped;
		}

		++__first1_unwrapped;

		__seek_possibly_wrapped_iterator(__first1_unwrapped, __result_unwrapped);
		return __first1_unwrapped;
	}
}

template <
	class _FirstUnwrappedForwardIterator_,
	class _SecondUnwrappedForwardIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedForwardIterator_ __find_end_unchecked(
	_FirstUnwrappedForwardIterator_		__first1_unwrapped,
	_FirstUnwrappedForwardIterator_		__last1_unwrapped,
	_SecondUnwrappedForwardIterator_	__first2_unwrapped,
	_SecondUnwrappedForwardIterator_	__last2_unwrapped,
	_Predicate_							__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>,
			type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>>)
{
	constexpr auto __is_random_access = type_traits::is_iterator_random_ranges_v<_FirstUnwrappedForwardIterator_> &&
		type_traits::is_iterator_random_ranges_v<_SecondUnwrappedForwardIterator_>;

	constexpr auto __is_bidirectional = type_traits::is_iterator_bidirectional_ranges_v<_FirstUnwrappedForwardIterator_> &&
		type_traits::is_iterator_bidirectional_ranges_v<_SecondUnwrappedForwardIterator_>;

	if constexpr (__is_random_access)
		return __find_end_unchecked_random_access(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, type_traits::__pass_function(__predicate));
	else if constexpr (__is_bidirectional)
		return __find_end_unchecked_bidirectional(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, type_traits::__pass_function(__predicate));
	else
		return __find_end_unchecked_forward(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
