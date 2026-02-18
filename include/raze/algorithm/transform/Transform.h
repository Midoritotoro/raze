#pragma once 

#include <src/raze/algorithm/unchecked/transform/TransformUnchecked.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _OutputIterator_,
    class _UnaryPredicate_>
__simd_nodiscard_inline_constexpr _OutputIterator_ transform(
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination,
    _UnaryPredicate_    __predicate) noexcept
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__destination, 
        __transform_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last),
        __unwrap_iterator(__destination), type_traits::__pass_function(__predicate)));

    return __destination;
}

template <
    class _FirstInputIterator_,
    class _SecondInputIterator_,
    class _OutputIterator_,
    class _UnaryPredicate_>
__simd_nodiscard_inline_constexpr _OutputIterator_ transform(
    _FirstInputIterator_    __first1,
    _FirstInputIterator_    __last1,
    _SecondInputIterator_   __first2,
    _OutputIterator_        __destination,
    _UnaryPredicate_        __predicate) noexcept
{
    __verify_range(__first1, __last1);

    __seek_possibly_wrapped_iterator(__destination, 
        __transform_unchecked(__unwrap_iterator(__first1), __unwrap_iterator(__last1),
        __unwrap_iterator(__first2), __unwrap_iterator(__destination), type_traits::__pass_function(__predicate)));

    return __destination;
}

__RAZE_ALGORITHM_NAMESPACE_END
