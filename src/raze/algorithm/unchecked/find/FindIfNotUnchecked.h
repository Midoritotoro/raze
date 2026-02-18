#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <
	class _InputUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputUnwrappedIterator_ __find_if_not_unchecked(
	_InputUnwrappedIterator_	__first_unwrapped,
	_InputUnwrappedIterator_	__last_unwrapped,
	_Predicate_					__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
		_Predicate_, type_traits::iterator_value_type<_InputUnwrappedIterator_>>)
{
	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		if (__predicate(*__first_unwrapped) == false)
			break;

	return __first_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
