#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _OutputIterator_,
    class _Predicate_>
__simd_inline_constexpr _OutputIterator_ __copy_if_unchecked(
    _InputIterator_     __first_unwrapped,
    _InputIterator_     __last_unwrapped,
    _OutputIterator_    __destination_unwrapped,
    _Predicate_         __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
    for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
        if (__predicate(*__first_unwrapped))
            *__destination_unwrapped++ = *__first_unwrapped;

    return __destination_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
