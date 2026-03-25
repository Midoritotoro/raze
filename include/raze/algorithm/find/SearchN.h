#pragma once 

#include <src/raze/algorithm/unchecked/find/SearchNUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _DifferenceType_,
    class _Type_,
    class _BinaryPredicate_>
__simd_nodiscard_inline_constexpr _ForwardIterator_ search_n(
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _DifferenceType_    __count,
    const _Type_&       __value,
    _BinaryPredicate_   __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<_BinaryPredicate_,
        type_traits::iterator_value_type<_ForwardIterator_>, _Type_>)

{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__first, __search_n_unchecked(
        __unwrap_iterator(__first), __unwrap_iterator(__last), __count,
        __value, type_traits::__pass_function(__predicate)));
    
    return __first;
}

template <
    class _ForwardIterator_,
    class _DifferenceType_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>>
__simd_nodiscard_inline_constexpr _ForwardIterator_ search_n(
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _DifferenceType_    __count,
    const _Type_&       __value) noexcept(
        type_traits::is_nothrow_invocable_v<type_traits::equal_to<>,
        type_traits::iterator_value_type<_ForwardIterator_>, _Type_>)
{
    return raze::algorithm::search_n(__first, __last, __count, __value, type_traits::equal_to<>{});
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _DifferenceType_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _ForwardIterator_ search_n(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _DifferenceType_    __count,
    const _Type_&       __value) noexcept(
        type_traits::is_nothrow_invocable_v<type_traits::equal_to<>,
        type_traits::iterator_value_type<_ForwardIterator_>, _Type_>)
{
    return raze::algorithm::search_n(__first, __last, __count, __value);
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _DifferenceType_,
    class _Type_,
    class _BinaryPredicate_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
__simd_nodiscard_inline_constexpr _ForwardIterator_ search_n(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _DifferenceType_    __count,
    const _Type_&       __value,
    _BinaryPredicate_   __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<_BinaryPredicate_,
        type_traits::iterator_value_type<_ForwardIterator_>, _Type_>)
{
    return raze::algorithm::search_n(__first, __last, __count, __value, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
