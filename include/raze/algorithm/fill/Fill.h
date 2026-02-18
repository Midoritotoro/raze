#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <raze/compatibility/Nodiscard.h>
#include <raze/compatibility/Inline.h>

#include <src/raze/algorithm/vectorized/fill/FillVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>

#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>>
__simd_inline_constexpr _ForwardIterator_ fill(
    _ForwardIterator_                                   __first,
    _ForwardIterator_                                   __last,
    const typename std::type_identity<_Type_>::type&    __value) noexcept
{
    __verify_range(__first, __last);

    auto __first_unwrapped         = __unwrap_iterator(__first);
    const auto __last_unwrapped    = __unwrap_iterator(__last);

    if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_ForwardIterator_, _Type_>) {
        const auto __difference = __iterators_difference(__first_unwrapped, __last_unwrapped);
        __memset_vectorized<_Type_>(std::to_address(__first_unwrapped), __value, __difference);
    }
    else
        for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
            *__first_unwrapped = __value;

    return __last;
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _Type_>
_ForwardIterator_ fill(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    const _Type_&       __value) noexcept
{
    return raze::algorithm::fill(__first, __last, __value);
}

__RAZE_ALGORITHM_NAMESPACE_END
