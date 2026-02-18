#pragma once 


#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/replace/ReplaceCopyVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _UnwrappedInputIterator_,
    class _UnwrappedOutputIterator_,
    class _UnaryPredicate_,
    class _Type_ = type_traits::iterator_value_type<_UnwrappedInputIterator_>>
__simd_inline_constexpr raze_always_inline void __replace_copy_if_unchecked(
    _UnwrappedInputIterator_                            __first_unwrapped,
    _UnwrappedInputIterator_                            __last_unwrapped,
    _UnwrappedOutputIterator_                           __destination_unwrapped,
    _UnaryPredicate_                                    __predicate,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept(
        type_traits::is_nothrow_invocable_v<_UnaryPredicate_, _Type_>)
{
    for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped, ++__destination_unwrapped)
        *__destination_unwrapped = __predicate(*__first_unwrapped) ? __new_value : *__first_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
