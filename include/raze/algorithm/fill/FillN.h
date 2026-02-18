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
    class _SizeType_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>>
__simd_inline_constexpr _ForwardIterator_ fill_n(
    _ForwardIterator_                                   __first,
    _SizeType_                                          __count,
    const typename std::type_identity<_Type_>::type&    __value) noexcept
{
    auto __first_unwrapped = __unwrap_iterator_offset(__first, __count);

    if constexpr (type_traits::__is_vectorized_find_algorithm_safe_v<_ForwardIterator_, _Type_>)
        __memset_vectorized<_Type_>(std::to_address(__first_unwrapped), __value, __count);
    else
        for (auto __current = _SizeType_(0); __current < __count; ++__current, ++__first_unwrapped)
            *__first_unwrapped = __value;

    __seek_possibly_wrapped_iterator(__first, __first + __count);
    return __first;
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _SizeType_,
    class _Type_>
__simd_inline_constexpr _ForwardIterator_ fill_n(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _SizeType_          __count,
    const _Type_&       __value) noexcept
{
    return raze::algorithm::fill_n(__first, __count, __value);
}

__RAZE_ALGORITHM_NAMESPACE_END
