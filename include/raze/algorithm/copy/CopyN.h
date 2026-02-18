#pragma once 

#include <raze/algorithm/copy/Copy.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _SizeType_,
    class _OutputIterator_>
__simd_inline_constexpr _OutputIterator_ copy_n(
    _InputIterator_     __first,
    _SizeType_          __elements_count,
    _OutputIterator_    __destination) noexcept
{
    return raze::algorithm::copy(__first, __first + __elements_count, __destination);
}

template <
    class _ExecutionPolicy_,
    class _InputIterator_,
    class _SizeType_,
    class _OutputIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
_OutputIterator_ copy_n(
    _ExecutionPolicy_&&,
    _InputIterator_     __first,
    _SizeType_          __elements_count,
    _OutputIterator_    __destination) noexcept
{
    return raze::algorithm::copy_n(__first, __elements_count, __destination);
}

__RAZE_ALGORITHM_NAMESPACE_END