#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/find/CountVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _UnwrappedIterator_,
	class _Type_ = type_traits::iterator_value_type<_UnwrappedIterator_>>
__simd_nodiscard_inline_constexpr sizetype __count_unchecked(
	_UnwrappedIterator_									__first_unwrapped,
	_UnwrappedIterator_									__last_unwrapped,
	const typename std::type_identity<_Type_>::type&	__value) noexcept
{
	using _DifferenceType = type_traits::iterator_difference_type<_UnwrappedIterator_>;

	if constexpr (type_traits::is_iterator_random_ranges_v<_UnwrappedIterator_>) {
		const auto __size = __byte_length(__first_unwrapped, __last_unwrapped);

		if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedIterator_, _Type_>) {
#if raze_has_cxx20
			if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
			{
				return __count_vectorized(std::to_address(__first_unwrapped), __size, __value);
			}
		}
	}

    auto __count = _DifferenceType(0);

	for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
		if (*__first_unwrapped == __value)
			++__count;

	return __count;
}


__RAZE_ALGORITHM_NAMESPACE_END
