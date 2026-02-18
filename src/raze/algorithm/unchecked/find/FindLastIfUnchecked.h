#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _InputUnwrappedIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputUnwrappedIterator_ __find_last_if_unchecked(
	_InputUnwrappedIterator_	__first_unwrapped,
	_InputUnwrappedIterator_	__last_unwrapped,
	_Predicate_					__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputUnwrappedIterator_>>)
{
	const auto __cached_last = __last_unwrapped;

	while (__last_unwrapped != __first_unwrapped) {
		--__last_unwrapped;

		if (__predicate(*__last_unwrapped))
			return __last_unwrapped;
	}

	return __cached_last;
}

__RAZE_ALGORITHM_NAMESPACE_END
