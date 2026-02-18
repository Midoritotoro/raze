#pragma once 

#include <src/raze/algorithm/vectorized/order/ReverseVectorized.h>
#include <raze/algorithm/swap/Swap.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _BidirectionalIterator_>
__simd_inline_constexpr void reverse(
    _BidirectionalIterator_ __first,
    _BidirectionalIterator_ __last) noexcept
{
    using _BidirectionalUnwrappedIterator_ = __unwrapped_iterator_type<_BidirectionalIterator_>;
    using _ValueType = type_traits::iterator_value_type<_BidirectionalUnwrappedIterator_>;

    __verify_range(__first, __last);

    auto __first_unwrapped = __unwrap_iterator(__first);
    auto __last_unwrapped  = __unwrap_iterator(__last);

    if constexpr (
        type_traits::is_iterator_contiguous_v<_BidirectionalUnwrappedIterator_> &&
        type_traits::__is_vector_type_supported_v<_ValueType>)
    {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            return __reverse_vectorized<_ValueType>(std::to_address(__first_unwrapped), std::to_address(__last_unwrapped));
        }
    }

    for (; __first_unwrapped != __last_unwrapped && __first_unwrapped != --__last_unwrapped; ++__first_unwrapped)
        raze::algorithm::iter_swap(__first_unwrapped, __last_unwrapped);
}

template <
    class _ExecutionPolicy_,
    class _BidirectionalIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_inline_constexpr void reverse(
    _ExecutionPolicy_&&,
    _BidirectionalIterator_ __first,
    _BidirectionalIterator_ __last) noexcept
{
    return raze::algorithm::reverse(__first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END
