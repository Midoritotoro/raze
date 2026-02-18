#pragma once 

#include <src/raze/algorithm/unchecked/remove/RemoveCopyUnchecked.h>
#include <src/raze/algorithm/unchecked/remove/RemoveCopyIfUnchecked.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _InputIterator_,
	class _OutputIterator_,
	class _Type_ = type_traits::iterator_value_type<_InputIterator_>>
__simd_nodiscard_inline_constexpr _OutputIterator_ remove_copy(
	_InputIterator_										__first,
	_InputIterator_										__last,
	_OutputIterator_									__destination,
	const typename std::type_identity<_Type_>::type&	__value) noexcept
{
	__verify_range(__first, __last);
	
	__seek_possibly_wrapped_iterator(__destination, __remove_copy_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), __unwrap_iterator(__destination), __value));

	return __destination;
}

template <
	class _InputIterator_,
	class _OutputIterator_,
	class _UnaryPredicate_>
__simd_nodiscard_inline_constexpr _OutputIterator_ remove_copy_if(
	_InputIterator_		__first,
	_InputIterator_		__last,
	_OutputIterator_	__destination,
	_UnaryPredicate_	__predicate) noexcept
{
	__verify_range(__first, __last);
	
	__seek_possibly_wrapped_iterator(__destination, __remove_copy_if_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), __unwrap_iterator(__destination), type_traits::__pass_function(__predicate)));

	return __destination;
}

template <
	class _ExecutionPolicy_,
	class _InputIterator_,
	class _OutputIterator_,
	class _Type_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline _OutputIterator_ remove_copy(
	_ExecutionPolicy_&&,
	_InputIterator_		__first,
	_InputIterator_		__last,
	_OutputIterator_	__destination,
	const _Type_&		__value) noexcept
{
	return raze::algorithm::remove_copy(__first, __last, __destination, __value);
}

template <
	class _ExecutionPolicy_,
	class _InputIterator_,
	class _OutputIterator_,
	class _UnaryPredicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline _OutputIterator_ remove_copy_if(
	_ExecutionPolicy_&&,
	_InputIterator_		__first,
	_InputIterator_		__last,
	_OutputIterator_	__destination,
	_UnaryPredicate_	__predicate) noexcept
{
	return raze::algorithm::remove_copy_if(__first, __last, __destination, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
