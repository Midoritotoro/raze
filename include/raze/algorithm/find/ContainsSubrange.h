#pragma once 

#include <src/raze/algorithm/unchecked/find/ContainsSubrangeUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr bool contains_subrange(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2,
	_Predicate_				__predicate) noexcept(
		std::is_nothrow_invocable_v<
		_Predicate_,
		std::iter_value_t<_FirstForwardIterator_>,
		std::iter_value_t<_SecondForwardIterator_>>)
{
	__verify_range(__first1, __last1);
	__verify_range(__first2, __last2);

	return __contains_subrange_unchecked(__unwrap_iterator(__first1),
		__unwrap_iterator(__last1), __unwrap_iterator(__first2), __unwrap_iterator(__last2),
		type_traits::__pass_function(__predicate));
}

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
__simd_nodiscard_inline_constexpr bool contains_subrange(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2) noexcept(
		std::is_nothrow_invocable_v<
		type_traits::equal_to<>,
		std::iter_value_t<_FirstForwardIterator_>,
		std::iter_value_t<_SecondForwardIterator_>>)
{
	return raze::algorithm::contains_subrange(__first1, __last1, __first2, __last2, type_traits::equal_to<>{});
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr bool contains_subrange(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2,
	_Predicate_				__predicate) noexcept(
		std::is_nothrow_invocable_v<
		_Predicate_,
		std::iter_value_t<_FirstForwardIterator_>,
		std::iter_value_t<_SecondForwardIterator_>>)
{
	return raze::algorithm::contains_subrange(__first1, __last1, __first2, __last2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr bool contains_subrange(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2,
	_SecondForwardIterator_ __last2) noexcept(
		std::is_nothrow_invocable_v<
		type_traits::equal_to<>,
		std::iter_value_t<_FirstForwardIterator_>,
		std::iter_value_t<_SecondForwardIterator_>>)
{
	return raze::algorithm::contains_subrange(__first1, __last1, __first2, __last2);
}


__RAZE_ALGORITHM_NAMESPACE_END
