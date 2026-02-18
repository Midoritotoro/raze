#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/replace/ReplaceVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _UnwrappedForwardIterator_,
    class _Type_ = type_traits::iterator_value_type<_UnwrappedForwardIterator_>>
__simd_inline_constexpr void __replace_unchecked(
    _UnwrappedForwardIterator_                          __first_unwrapped,
    _UnwrappedForwardIterator_                          __last_unwrapped,
    const typename std::type_identity<_Type_>::type&    __old_value,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept
{
    if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedForwardIterator_, _Type_>) {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            return __replace_vectorized(std::to_address(__first_unwrapped),
                std::to_address(__last_unwrapped), __old_value, __new_value);
        }
    }

    for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
        if (*__first_unwrapped == __old_value)
            *__first_unwrapped = __new_value;
}

__RAZE_ALGORITHM_NAMESPACE_END
