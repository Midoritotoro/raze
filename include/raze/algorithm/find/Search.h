#pragma once 

#include <src/raze/algorithm/unchecked/find/SearchUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_> 
__simd_nodiscard_inline_constexpr _FirstForwardIterator_ search(
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
	__verify_range(__first1, __last1);
	__verify_range(__first2, __last2);
	
	__seek_possibly_wrapped_iterator(__first1, __search_unchecked(__unwrap_iterator(__first1),
		__unwrap_iterator(__last1), __unwrap_iterator(__first2), __unwrap_iterator(__last2),
		type_traits::__pass_function(__predicate)));

	return __first1;
}

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
__simd_nodiscard_inline_constexpr _FirstForwardIterator_ search(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::search(__first1, __last1, __first2, __last2, type_traits::equal_to<>{});
}

template <
	class _ForwardIterator_, 
	class _Searcher_>
__simd_nodiscard_inline_constexpr _ForwardIterator_ search(
	_ForwardIterator_ __first, 
	_ForwardIterator_ __last,
	const _Searcher_& __searcher) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Searcher_, type_traits::iterator_value_type<_ForwardIterator_>>)
{
	return __searcher(__first, __last).first;
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _FirstForwardIterator_ search(
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
	return raze::algorithm::search(__first1, __last1, __first2, __last2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _FirstForwardIterator_ search(
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
	return raze::algorithm::search(__first1, __last1, __first2, __last2);
}


__RAZE_ALGORITHM_NAMESPACE_END
