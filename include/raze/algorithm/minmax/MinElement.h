#pragma once 

#include <src/raze/algorithm/unchecked/minmax/MinElementUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _Predicate_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _ForwardIterator_ min_element(
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _Predicate_         __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<_Predicate_,
        type_traits::iterator_value_type<_ForwardIterator_>,
        type_traits::iterator_value_type<_ForwardIterator_>>)
{
    __verify_range(__first, __last);
    __seek_possibly_wrapped_iterator(__first, __min_element_unchecked(
        __unwrap_iterator(__first), __unwrap_iterator(__last),
        type_traits::__pass_function(__predicate)));

    return __first;
}

template <class _ForwardIterator_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _ForwardIterator_ min_element(
    _ForwardIterator_ __first,
    _ForwardIterator_ __last) noexcept(
        type_traits::is_nothrow_invocable_v<type_traits::less<>,
        type_traits::iterator_value_type<_ForwardIterator_>,
        type_traits::iterator_value_type<_ForwardIterator_>>)
{
    return raze::algorithm::min_element(__first, __last, type_traits::less<>{});
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline _ForwardIterator_ min_element(
    _ExecutionPolicy_&&,
    _ForwardIterator_ __first,
    _ForwardIterator_ __last)
{
    return raze::algorithm::min_element(__first, __last);
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _Predicate_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline _ForwardIterator_ min_element(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _Predicate_         __predicate)
{
    return raze::algorithm::min_element(__first, __last, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
