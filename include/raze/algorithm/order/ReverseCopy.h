#pragma once 

#include <src/raze/algorithm/vectorized/order/ReverseCopyVectorized.h>

#include <raze/algorithm/swap/Swap.h>
#include <raze/memory/Intersects.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _FirstBidirectionalIterator_,
    class _SecondBidirectionalIterator_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline void reverse_copy(
    _FirstBidirectionalIterator_    __first,
    _FirstBidirectionalIterator_    __last,
    _SecondBidirectionalIterator_   __destination) noexcept
{
    using _FirstBidirectionalUnwrappedIterator_     = __unwrapped_iterator_type<_FirstBidirectionalIterator_>;
    using _SecondBidirectionalUnwrappedIterator_    = __unwrapped_iterator_type<_SecondBidirectionalIterator_>;

    using _FirstBidirectionalIteratorValueType_     = type_traits::iterator_value_type<_FirstBidirectionalUnwrappedIterator_>;

    __verify_range(__first, __last);
    memory::__check_intersection(__first, __last, __destination);

    auto __first_unwrapped          = __unwrap_iterator(__first);
    auto __last_unwrapped           = __unwrap_iterator(__last);

    auto __destination_unwrapped    = __unwrap_iterator(__destination);

    if constexpr (
        type_traits::is_iterator_contiguous_v<_FirstBidirectionalUnwrappedIterator_> &&
        type_traits::is_iterator_contiguous_v<_SecondBidirectionalUnwrappedIterator_> &&
        type_traits::__is_vector_type_supported_v<_FirstBidirectionalIteratorValueType_>)
    {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
        {
            return __reverse_copy_vectorized<_FirstBidirectionalIteratorValueType_>(
                std::to_address(__first_unwrapped), std::to_address(__last_unwrapped),
                std::to_address(__destination_unwrapped));
        }
    }

    for (; __first_unwrapped != __last_unwrapped; ++__destination_unwrapped)
        *__destination_unwrapped = std::move(*--__last_unwrapped);
}

template <
    class _ExecutionPolicy_,
    class _FirstBidirectionalIterator_,
    class _SecondBidirectionalIterator_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline void reverse_copy(
    _ExecutionPolicy_&&,
    _FirstBidirectionalIterator_    __first,
    _FirstBidirectionalIterator_    __last,
    _SecondBidirectionalIterator_   __destination) noexcept
{
    return raze::algorithm::reverse_copy(__first, __last, __destination);
}

__RAZE_ALGORITHM_NAMESPACE_END
