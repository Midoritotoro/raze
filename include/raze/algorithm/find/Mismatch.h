#pragma once 

#include <src/raze/algorithm/unchecked/find/MismatchUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstIterator_,
	class _SecondIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_Predicate_			__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	__verify_range(__first1, __last1);

	const auto __mismatch_position = __mismatch_unchecked(__unwrap_iterator(__first1), __unwrap_iterator(__last1),
		__unwrap_iterator(__first2), type_traits::__pass_function(__predicate));

	__seek_possibly_wrapped_iterator(__first1, __mismatch_position.first);
	__seek_possibly_wrapped_iterator(__first2, __mismatch_position.second);

	return { __first1, __first2 };
}

template <
	class _FirstIterator_,
	class _SecondIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_SecondIterator_	__last2,
	_Predicate_			__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	__verify_range(__first1, __last1);
	__verify_range(__first2, __last2);

	const auto __mismatch_position = __mismatch_unchecked(__unwrap_iterator(__first1), __unwrap_iterator(__last1),
		__unwrap_iterator(__first2), __unwrap_iterator(__last2), type_traits::__pass_function(__predicate));

	__seek_possibly_wrapped_iterator(__first1, __mismatch_position.first);
	__seek_possibly_wrapped_iterator(__first2, __mismatch_position.second);

	return { __first1, __first2 };
}

template <
	class _FirstIterator_,
	class _SecondIterator_>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2, type_traits::equal_to<>{});
}

template <
	class _FirstIterator_,
	class _SecondIterator_>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_SecondIterator_	__last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2, __last2, type_traits::equal_to<>{});
}

template <
	class _ExecutionPolicy_,
	class _FirstIterator_,
	class _SecondIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_ExecutionPolicy_&&,
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_Predicate_			__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstIterator_,
	class _SecondIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_ExecutionPolicy_&&,
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_SecondIterator_	__last2,
	_Predicate_			__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2, __last2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstIterator_,
	class _SecondIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_ExecutionPolicy_&&,
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2);
}

template <
	class _ExecutionPolicy_,
	class _FirstIterator_,
	class _SecondIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr std::pair<_FirstIterator_, _SecondIterator_> mismatch(
	_ExecutionPolicy_&&,
	_FirstIterator_		__first1,
	_FirstIterator_		__last1,
	_SecondIterator_	__first2,
	_SecondIterator_	__last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstIterator_>,
			type_traits::iterator_value_type<_SecondIterator_>>)
{
	return raze::algorithm::mismatch(__first1, __last1, __first2, __last2);
}

__RAZE_ALGORITHM_NAMESPACE_END
