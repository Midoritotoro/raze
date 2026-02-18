#pragma once 

#include <src/raze/algorithm/vectorized/transform/TransformVectorized.h>
#include <src/raze/algorithm/AlgorithmDebug.h>

#include <src/raze/type_traits/SimdAlgorithmSafety.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _UnwrappedInputIterator_,
    class _UnwrappedOutputIterator_,
    class _UnaryPredicate_>
__simd_nodiscard_inline_constexpr _UnwrappedOutputIterator_ __transform_unchecked(
    _UnwrappedInputIterator_    __first_unwrapped,
    _UnwrappedInputIterator_    __last_unwrapped,
    _UnwrappedOutputIterator_   __destination_unwrapped,
    _UnaryPredicate_            __predicate) noexcept
{
    using _ValueType = type_traits::iterator_value_type<_UnwrappedInputIterator_>;

    constexpr auto __is_vectorizable = type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedInputIterator_, _ValueType>
        && type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedOutputIterator_, _ValueType>;

    if constexpr (__is_vectorizable) {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            auto __destination_address = std::to_address(__destination_unwrapped);

            const auto __destination_last = __transform_vectorized<_ValueType>(std::to_address(__first_unwrapped),
                std::to_address(__last_unwrapped), __destination_address, type_traits::__pass_function(__predicate));
            
            if constexpr (std::is_pointer_v<_UnwrappedOutputIterator_>)
                return __destination_last;
            else
                return __destination_unwrapped + (__destination_last - __destination_address);
        }
    }

    for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped, ++__destination_unwrapped)
        *__destination_unwrapped = __predicate(*__first_unwrapped);

    return __destination_unwrapped;
}

template <
    class _UnwrappedFirstInputIterator_,
    class _UnwrappedSecondInputIterator_,
    class _UnwrappedOutputIterator_,
    class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr _UnwrappedOutputIterator_ __transform_unchecked(
    _UnwrappedFirstInputIterator_   __first1_unwrapped,
    _UnwrappedFirstInputIterator_   __last1_unwrapped,
    _UnwrappedSecondInputIterator_  __first2_unwrapped,
    _UnwrappedOutputIterator_       __destination_unwrapped,
    _BinaryPredicate_               __predicate) noexcept
{
    using _ValueType = type_traits::iterator_value_type<_UnwrappedFirstInputIterator_>;

    constexpr auto __is_vectorizable = type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedFirstInputIterator_, _ValueType>
        && type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedOutputIterator_, _ValueType>
        && type_traits::__is_vectorized_find_algorithm_safe_v<_UnwrappedSecondInputIterator_, _ValueType>;

    if constexpr (__is_vectorizable) {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            auto __destination_address = std::to_address(__destination_unwrapped);

            const auto __destination_last = __transform_vectorized<_ValueType>(std::to_address(__first1_unwrapped),
                std::to_address(__last1_unwrapped), std::to_address(__first2_unwrapped),
                __destination_address, type_traits::__pass_function(__predicate));
            
            if constexpr (std::is_pointer_v<_UnwrappedOutputIterator_>)
                return __destination_last;
            else
                return __destination_unwrapped + (__destination_last - __destination_address);
        }
    }

    for (; __first1_unwrapped != __last1_unwrapped; ++__first1_unwrapped, ++__first2_unwrapped, ++__destination_unwrapped)
        *__destination_unwrapped = __predicate(*__first1_unwrapped, *__first2_unwrapped);

    return __destination_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
