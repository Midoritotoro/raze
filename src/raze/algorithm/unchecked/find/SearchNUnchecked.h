#pragma once 

#include <src/raze/type_traits/SimdAlgorithmSafety.h>
#include <src/raze/algorithm/vectorized/find/SearchNVectorized.h>
#include <raze/algorithm/find/Find.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardUnwrappedIterator_, 
    class _DifferenceType_,
    class _Type_,
    class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr _ForwardUnwrappedIterator_ __search_n_unchecked(
    _ForwardUnwrappedIterator_  __first_unwrapped,
    _ForwardUnwrappedIterator_  __last_unwrapped,
    _DifferenceType_            __count, 
    const _Type_&               __value,
    _BinaryPredicate_           __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<_BinaryPredicate_,
        type_traits::iterator_value_type<_ForwardUnwrappedIterator_>, _Type_>)
{
    if (__count <= 0)
        return __first_unwrapped;

	if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_ForwardUnwrappedIterator_, _Type_>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			const auto __first_address = std::to_address(__first_unwrapped);
			const auto __position = __search_n_vectorized(__first_address, std::to_address(__last_unwrapped), __count, __value);

			if constexpr (std::is_pointer_v<_ForwardUnwrappedIterator_>)
				return __position;
			else
				return __first_unwrapped + (__position - __first_address);
		}
	}

    auto __found_begin = __first_unwrapped;

    while (true) {
        __found_begin = raze::algorithm::find_if(__found_begin, __last_unwrapped,
            [__value, &__predicate] (const auto& __x) { 
                return __predicate(__x, __value); 
            });

        if (__found_begin == __last_unwrapped)
            return __last_unwrapped;

        auto __run = __found_begin;
        auto __matched = _DifferenceType_(0);

        while (__run != __last_unwrapped && __predicate(*__run, __value) && __matched < __count) {
            ++__run; 
            ++__matched;
        }

        if (__matched == __count)
            return __found_begin;

        __found_begin = std::next(__found_begin);
    }

}

__RAZE_ALGORITHM_NAMESPACE_END
