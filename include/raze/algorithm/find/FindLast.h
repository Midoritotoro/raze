#pragma once 

#include <src/raze/algorithm/unchecked/find/FindLastUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindLastIfUnchecked.h>

#include <src/raze/algorithm/unchecked/find/FindLastIfNotUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _Iterator_,
	class _Type_ = type_traits::iterator_value_type<_Iterator_>>
__simd_nodiscard_inline_constexpr _Iterator_ find_last(
	_Iterator_		__first,
	_Iterator_		__last,
	const _Type_&	__value) noexcept
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_last_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last), __value));
	return __first;
}

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_last_if_not(
	_InputIterator_	__first,
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_last_if_not_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last),
		type_traits::__pass_function(__predicate)));

	return __first;
}

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_last_if(
	_InputIterator_	__first,
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
	__verify_range(__first, __last);
	
	__seek_possibly_wrapped_iterator(__first, __find_last_if_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last),
		type_traits::__pass_function(__predicate)));

	return __first;
}

template <
	class _ExecutionPolicy_,
	class _Iterator_,
	class _Type_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _Iterator_ find_last(
	_ExecutionPolicy_&&,
	_Iterator_		__first,
	_Iterator_		__last,
	const _Type_&	__value) noexcept
{
	return raze::algorithm::find_last(__first, __last, __value);
}

template <
	class _ExecutionPolicy_,
	class _InputIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _InputIterator_ find_last_if_not(
	_ExecutionPolicy_&&,
	_InputIterator_	__first,
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
	return raze::algorithm::find_last_if_not(__first, __last, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _InputIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _InputIterator_ find_last_if(
	_ExecutionPolicy_&&,
	_InputIterator_	__first, 
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
	return raze::algorithm::find_last_if(__first, __last, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
