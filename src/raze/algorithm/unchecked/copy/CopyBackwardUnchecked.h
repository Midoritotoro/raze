#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/copy/CopyVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _UnwrappedBidirectionalFirstIterator_,
    class _UnwrappedBidirectionalSecondIterator_>
__simd_inline_constexpr _UnwrappedBidirectionalSecondIterator_ __copy_backward_unchecked(
    _UnwrappedBidirectionalFirstIterator_    __first_unwrapped,
    _UnwrappedBidirectionalFirstIterator_    __last_unwrapped,
    _UnwrappedBidirectionalSecondIterator_   __destination_last_unwrapped) noexcept
{
    const auto __difference = __iterators_difference(__first_unwrapped, __last_unwrapped);

    if constexpr (type_traits::__copy_iterator_category<_UnwrappedBidirectionalFirstIterator_, _UnwrappedBidirectionalSecondIterator_>::BitcopyAssignable) {
#if raze_has_cxx20
        if (type_traits::is_constant_evaluated() == false)
#endif
        {
            auto __first_address    = std::to_address(__first_unwrapped);
            const auto __size       = __byte_length(__first_address, std::to_address(__last_unwrapped));

            __memcpy_vectorized(const_cast<char*>(reinterpret_cast<const volatile char*>(
                std::to_address(__destination_last_unwrapped))) - __size, __first_address, __size);

            return (__destination_last_unwrapped - __difference);
        }
    }

     while (__first_unwrapped != __last_unwrapped)
        *(--__destination_last_unwrapped) = *(--__last_unwrapped);

    return __destination_last_unwrapped;
}

__RAZE_ALGORITHM_NAMESPACE_END
