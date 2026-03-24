#pragma once 

#include <src/raze/algorithm/unchecked/find/AdjacentFindUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr _ForwardIterator_ adjacent_find(
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _BinaryPredicate_   __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _BinaryPredicate_, 
            type_traits::iterator_value_type<_ForwardIterator_>, 
            type_traits::iterator_value_type<_ForwardIterator_>>)
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__first, __adjacent_find_unchecked(
        __unwrap_iterator(__first), __unwrap_iterator(__last),
        type_traits::__pass_function(__predicate)));

    return __first;
}

template <class _ForwardIterator_>
__simd_nodiscard_inline_constexpr _ForwardIterator_ adjacent_find(
    _ForwardIterator_ __first,
    _ForwardIterator_ __last) noexcept
{
    return raze::algorithm::adjacent_find(__first, __last, type_traits::equal_to<>{});
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline _ForwardIterator_ adjacent_find(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last) noexcept
{
    return raze::algorithm::adjacent_find(__first, __last);
}

template <
    class _ExecutionPolicy_, 
    class _ForwardIterator_, 
    class _BinaryPredicate_>
__simd_nodiscard_inline _ForwardIterator_ adjacent_find(
    _ExecutionPolicy_&&,
    _ForwardIterator_ __first, 
    _ForwardIterator_ __last,
    _BinaryPredicate_ __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _BinaryPredicate_, 
            type_traits::iterator_value_type<_ForwardIterator_>, 
            type_traits::iterator_value_type<_ForwardIterator_>>)
{
    return raze::algorithm::adjacent_find(__first, __last, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
