#pragma once 

#include <src/raze/algorithm/unchecked/minmax/MinUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _InputIterator_>
__simd_nodiscard_inline type_traits::iterator_value_type<_InputIterator_> min_range(
	_InputIterator_ __first,
	_InputIterator_ __last) noexcept
{
	raze_debug_assert(__first != __last && "min_range requires non-empty range");
	return __min_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last));
}

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline type_traits::iterator_value_type<_InputIterator_> min_range(
	_InputIterator_ __first,
	_InputIterator_ __last,
	_Predicate_		__predicate) noexcept
{
	raze_debug_assert(__first != __last && "min_range requires non-empty range");
	return __min_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last), type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
