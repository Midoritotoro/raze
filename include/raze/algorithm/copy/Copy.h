#pragma once 

#include <src/raze/algorithm/unchecked/copy/CopyUnchecked.h>
#include <src/raze/algorithm/unchecked/copy/CopyIfUnchecked.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _OutputIterator_>
__simd_inline_constexpr _OutputIterator_ copy(
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination) noexcept
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__destination, __copy_unchecked(__unwrap_iterator(__first),
        __unwrap_iterator(__last), __unwrap_unverified_iterator(__destination)));

    return __destination;
}

template <
    class _InputIterator_,
    class _OutputIterator_,
    class _Predicate_>
__simd_inline_constexpr _OutputIterator_ copy_if(
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination,
    _Predicate_         __predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
    __verify_range(__first, __last);

    __seek_possibly_wrapped_iterator(__destination, __copy_if_unchecked(__unwrap_iterator(__first),
        __unwrap_iterator(__last), __unwrap_unverified_iterator(__destination),
        type_traits::__pass_function(__predicate)));

    return __destination;
}

template <
    class _ExecutionPolicy_,
    class _InputIterator_,
    class _OutputIterator_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
_OutputIterator_ copy(
    _ExecutionPolicy_&&,
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination) noexcept
{
    return raze::algorithm::copy(__first, __last, __destination);
}

template <
    class _ExecutionPolicy_,
    class _InputIterator_,
    class _OutputIterator_,
    class _Predicate_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
_OutputIterator_ copy_if(
    _ExecutionPolicy_&&,
    _InputIterator_     __first,
    _InputIterator_     __last,
    _OutputIterator_    __destination,
    _Predicate_         __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
    return raze::algorithm::copy_if(__first, __last, __destination, type_traits::__pass_function(__predicate));
}


__RAZE_ALGORITHM_NAMESPACE_END