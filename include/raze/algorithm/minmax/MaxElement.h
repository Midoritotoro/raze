#pragma once 

#include <src/raze/algorithm/unchecked/minmax/MaxElementUnchecked.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_, 
    class _Predicate_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _ForwardIterator_ max_element(
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _Predicate_         __predicate) noexcept(
        std::is_nothrow_invocable_v<_Predicate_,
            std::iter_value_t<_ForwardIterator_>, 
            std::iter_value_t<_ForwardIterator_>>)
{
    __verify_range(__first, __last);
    __seek_possibly_wrapped_iterator(__first, __max_element_unchecked(
        __unwrap_iterator(__first), __unwrap_iterator(__last),
        type_traits::__pass_function(__predicate)));

    return __first;
}

template <class _ForwardIterator_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _ForwardIterator_ max_element(
    _ForwardIterator_ __first, 
    _ForwardIterator_ __last) noexcept(
        std::is_nothrow_invocable_v<type_traits::less<>,
            std::iter_value_t<_ForwardIterator_>, 
            std::iter_value_t<_ForwardIterator_>>)
{
    return raze::algorithm::max_element(__first, __last, type_traits::less<>{});
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline _ForwardIterator_ max_element(
    _ExecutionPolicy_&&,
    _ForwardIterator_ __first,
    _ForwardIterator_ __last)
{
    return raze::algorithm::max_element(__first, __last);
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _Predicate_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_always_inline _ForwardIterator_ max_element(
    _ExecutionPolicy_&&,
    _ForwardIterator_   __first,
    _ForwardIterator_   __last,
    _Predicate_         __predicate)
{
    return raze::algorithm::max_element(__first, __last, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
