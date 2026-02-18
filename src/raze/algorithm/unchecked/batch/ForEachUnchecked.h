#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedInputIterator_,
	class _UnaryFunction_>
__simd_inline_constexpr void __for_each_unchecked(
	_UnwrappedInputIterator_	__first_unwrapped,
	_UnwrappedInputIterator_	__last_unwrapped,
	_UnaryFunction_				__function) noexcept(
		type_traits::is_nothrow_invocable_v<_UnaryFunction_,
			type_traits::iterator_value_type<_UnwrappedInputIterator_>>)
{
	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		__function(*__first_unwrapped);
}

__RAZE_ALGORITHM_NAMESPACE_END
