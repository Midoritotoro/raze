#pragma once 

#include <src/raze/algorithm/unchecked/copy/MoveBackwardUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _BidirectionalFirstIterator_,
    class _BidirectionalSecondIterator_>
__simd_inline_constexpr _BidirectionalSecondIterator_ move_backward(
    _BidirectionalFirstIterator_    __first,
    _BidirectionalFirstIterator_    __last,
    _BidirectionalSecondIterator_   __destination_last) noexcept
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__destination_last, __move_backward_unchecked(__unwrap_iterator(__first),
        __unwrap_iterator(__last), __unwrap_unverified_iterator(__destination_last)));

    return __destination_last;
}

template <
    class _ExecutionPolicy_,
    class _BidirectionalFirstIterator_,
    class _BidirectionalSecondIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
_BidirectionalSecondIterator_ move_backward(
    _ExecutionPolicy_&&,
    _BidirectionalFirstIterator_    __first,
    _BidirectionalFirstIterator_    __last,
    _BidirectionalSecondIterator_   __destination_last) noexcept
{
    return raze::algorithm::move_backward(__first, __last, __destination_last);
}

__RAZE_ALGORITHM_NAMESPACE_END