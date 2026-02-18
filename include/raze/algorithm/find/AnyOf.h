#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _InputIterator_, 
    class _Predicate_>
__simd_inline_constexpr bool any_of(
    _InputIterator_ __first,
    _InputIterator_ __last,
    _Predicate_     __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
		    _Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
    return (raze::algorithm::find_if(__first, __last, type_traits::__pass_function(__predicate)) != __last);
}

template <
    class _ExecutionPolicy_,
    class _InputIterator_,
    class _Predicate_,
    concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline bool any_of(
    _ExecutionPolicy_&&,
    _InputIterator_ __first,
    _InputIterator_ __last,
    _Predicate_     __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _Predicate_, type_traits::iterator_value_type<_InputIterator_>>)
{
    return raze::algorithm::any_of(__first, __last, type_traits::__pass_function(__predicate));
}

__RAZE_ALGORITHM_NAMESPACE_END
