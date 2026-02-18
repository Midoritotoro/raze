#pragma once 


#include <src/raze/algorithm/unchecked/replace/ReplaceIfUnchecked.h>
#include <src/raze/algorithm/unchecked/replace/ReplaceUnchecked.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>>
raze_constexpr_cxx20 raze_always_inline void replace(
    _ForwardIterator_                                   __first,
    _ForwardIterator_                                   __last,
    const typename std::type_identity<_Type_>::type&    __old_value,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept
{
    __verify_range(__first, __last);
    __replace_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last), __old_value, __new_value);
}

template <
    class _ForwardIterator_,
    class _UnaryPredicate_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>>
raze_constexpr_cxx20 raze_always_inline void replace_if(
    _ForwardIterator_                                   __first,
    _ForwardIterator_                                   __last,
    _UnaryPredicate_                                    __predicate,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept(
        type_traits::is_nothrow_invocable_v<_UnaryPredicate_, _Type_>)
{
    __verify_range(__first, __last);
    __replace_if_unchecked(__unwrap_iterator(__first), __unwrap_iterator(__last),
        type_traits::__pass_function(__predicate), __new_value);
}


template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_constexpr_cxx20 raze_always_inline void replace(
    _ExecutionPolicy_&&,
    _ForwardIterator_                                   __first,
    _ForwardIterator_                                   __last,
    const typename std::type_identity<_Type_>::type&    __old_value,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept
{
    return raze::algorithm::replace(__first, __last, __old_value, __new_value);
}

template <
    class _ExecutionPolicy_,
    class _ForwardIterator_,
    class _UnaryPredicate_,
    class _Type_ = type_traits::iterator_value_type<_ForwardIterator_>,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_constexpr_cxx20 raze_always_inline void replace_if(
    _ExecutionPolicy_&&,
    _ForwardIterator_                                   __first,
    _ForwardIterator_                                   __last,
    _UnaryPredicate_                                    __predicate,
    const typename std::type_identity<_Type_>::type&    __new_value) noexcept(
        type_traits::is_nothrow_invocable_v<_UnaryPredicate_, _Type_>)
{
    return raze::algorithm::replace_if(__first, __last, type_traits::__pass_function(__predicate), __new_value);
}

__RAZE_ALGORITHM_NAMESPACE_END
