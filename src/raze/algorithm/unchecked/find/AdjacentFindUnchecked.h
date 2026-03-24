#pragma once 

#include <src/raze/algorithm/vectorized/find/AdjacentFindVectorized.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIteratorUnwrapped_,
    class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr _ForwardIteratorUnwrapped_ __adjacent_find_unchecked(
    _ForwardIteratorUnwrapped_  __first_unwrapped,
    _ForwardIteratorUnwrapped_  __last_unwrapped,
    _BinaryPredicate_           __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _BinaryPredicate_, 
            type_traits::iterator_value_type<_ForwardIteratorUnwrapped_>,
            type_traits::iterator_value_type<_ForwardIteratorUnwrapped_>>)
{
    using _Type = type_traits::iterator_value_type<_ForwardIteratorUnwrapped_>;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_ForwardIteratorUnwrapped_, _Type>
        && type_traits::is_any_of_v<_BinaryPredicate_, type_traits::equal_to<>, std::equal_to<>>) 
    {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __first_address = std::to_address(__first_unwrapped);
			const auto __position = __adjacent_find_vectorized<_Type>(__first_address, std::to_address(__last_unwrapped));

			if constexpr (std::is_pointer_v<_ForwardIteratorUnwrapped_>)
				return __position;
			else
				return __first_unwrapped + (__position - __first_address);
		}
	}

    if (__first_unwrapped == __last_unwrapped)
        return __last_unwrapped;

    for (auto __next = __first_unwrapped; ++__next != __last_unwrapped; __first_unwrapped = __next)
        if (__predicate(*__first_unwrapped, *__next))
            return __first_unwrapped;

    return __last_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
