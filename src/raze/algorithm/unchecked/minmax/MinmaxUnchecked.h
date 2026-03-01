#pragma once 

#include <src/raze/algorithm/vectorized/minmax/MinmaxVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/type_traits/SimdAlgorithmSafety.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _UnwrappedInputIterator_>
using __minmax_return_type = std::pair<
	type_traits::iterator_value_type<_UnwrappedInputIterator_>,
	type_traits::iterator_value_type<_UnwrappedInputIterator_>>;

template <
	class _UnwrappedInputIterator_,
	class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr __minmax_return_type<_UnwrappedInputIterator_> __minmax_unchecked(
	_UnwrappedInputIterator_	__first_unwrapped,
	_UnwrappedInputIterator_	__last_unwrapped,
	_BinaryPredicate_			__predicate) noexcept
{
	using _ValueType	= type_traits::iterator_value_type<_UnwrappedInputIterator_>;

	auto __minmax = std::pair<_ValueType, _ValueType>(*__first_unwrapped, *__first_unwrapped);

    for (; ++__first_unwrapped != __last_unwrapped; ) {
        if (__predicate(*__first_unwrapped, __minmax.second))
			__minmax.second = *__first_unwrapped;
		if (__predicate(__minmax.first, *__first_unwrapped))
			__minmax.first = *__first_unwrapped;
    }

	return __minmax;
}

template <class _UnwrappedInputIterator_>
__simd_nodiscard_inline_constexpr __minmax_return_type<_UnwrappedInputIterator_> __minmax_unchecked(
	_UnwrappedInputIterator_ __first_unwrapped,
	_UnwrappedInputIterator_ __last_unwrapped) noexcept
{
	using _ValueType = type_traits::iterator_value_type<_UnwrappedInputIterator_>;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _ValueType>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			return __minmax_vectorized<_ValueType>(std::to_address(__first_unwrapped), std::to_address(__last_unwrapped));
		}
	}

	return raze::algorithm::__minmax_unchecked(__first_unwrapped, __last_unwrapped, type_traits::greater<>{});
}

__RAZE_ALGORITHM_NAMESPACE_END