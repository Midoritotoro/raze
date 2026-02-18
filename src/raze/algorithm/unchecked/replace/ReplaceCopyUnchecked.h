#pragma once 


#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/replace/ReplaceCopyVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _UnwrappedInputIterator_,
    class _UnwrappedOutputIterator_,
    class _Type_ = type_traits::iterator_value_type<_UnwrappedInputIterator_>>
__simd_inline_constexpr void __replace_copy_unchecked(
    _UnwrappedInputIterator_                            __first_unwrapped,
    _UnwrappedInputIterator_                            __last_unwrapped,
    _UnwrappedOutputIterator_                           __destination_unwrapped,
    const typename std::type_identity<_Type_>::type&    __old_value,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept
{
    constexpr auto __is_vectorizable = type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _Type_> &&
        type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedOutputIterator_, _Type_>;

    if constexpr (__is_vectorizable) {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            return __replace_copy_vectorized(std::to_address(__first_unwrapped),
                std::to_address(__last_unwrapped), std::to_address(__destination_unwrapped), 
                __old_value, __new_value);
        }
    }

    for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped, ++__destination_unwrapped)
        *__destination_unwrapped = (*__first_unwrapped == __old_value) ? __new_value : *__first_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
