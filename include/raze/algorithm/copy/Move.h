#pragma once 

#include <src/raze/algorithm/unchecked/copy/MoveUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _OutputIterator_>
__simd_inline_constexpr _OutputIterator_ move(
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination) noexcept
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__destination, __move_unchecked(__unwrap_iterator(__first),
        __unwrap_iterator(__last), __unwrap_unverified_iterator(__destination)));

    return __destination;
}

template <
    class _ExecutionPolicy_,
    class _InputIterator_,
    class _OutputIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
_OutputIterator_ move(
    _ExecutionPolicy_&&,
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination) noexcept
{
    return raze::algorithm::move(__first, __last, __destination);
}

__RAZE_ALGORITHM_NAMESPACE_END