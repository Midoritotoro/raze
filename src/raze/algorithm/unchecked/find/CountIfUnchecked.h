#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedInputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr type_traits::iterator_difference_type<_UnwrappedInputIterator_> __count_if_unchecked(
	_UnwrappedInputIterator_	__first_unwrapped,
	_UnwrappedInputIterator_	__last_unwrapped,
	_Predicate_ 				__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
		_Predicate_, type_traits::iterator_value_type<_UnwrappedInputIterator_>>)
{
	using _DifferenceType = type_traits::iterator_difference_type<_UnwrappedInputIterator_>;

	auto __count = _DifferenceType(0);

	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		if (__predicate(*__first_unwrapped))
			++__count;

	return __count;
}

__RAZE_ALGORITHM_NAMESPACE_END
