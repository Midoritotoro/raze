#pragma once 

#include <src/raze/algorithm/unchecked/minmax/MaxUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _InputIterator_,
	class _Type_>
__simd_nodiscard_inline _Type_ max_range(
	_InputIterator_ __first,
	_InputIterator_ __last) noexcept
{
	raze_debug_assert(__first != __last && "max_range requires non-empty range");
	return __max_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last));
}

template <
	class _InputIterator_,
	class _Type_,
	class _Predicate_>
__simd_nodiscard_inline _Type_ max_range(
	_InputIterator_ __first,
	_InputIterator_ __last,
	_Predicate_		__predicate) noexcept
{
	raze_debug_assert(__first != __last && "max_range requires non-empty range");
	return __max_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last), type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
