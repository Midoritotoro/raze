#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/RazeNamespace.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <xutility>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Iterator_>
raze_always_inline constexpr decltype(auto) __uiter(_Iterator_&& __iterator)
    noexcept(traits::__is_iterator_unwrappable_v<_Iterator_> == false || traits::__is_nothrow_unwrappable_v<_Iterator_>)
{
    if constexpr (std::is_pointer_v<std::decay_t<_Iterator_>>) return __iterator + 0;
    else if constexpr (traits::__is_iterator_unwrappable_v<_Iterator_>) return std::move(__iterator)._Unwrapped();
    else return std::move(__iterator);
}

template <class _Iterator_>
using __unwrapped_iterator_type = std::remove_cvref_t<decltype(__uiter(std::declval<_Iterator_>()))>;

template <class _Iterator_, class _UnwrappedIterator_>
raze_always_inline constexpr void __seek_iter(
    _Iterator_& __iterator, _UnwrappedIterator_&& __unwrapped_iterator) noexcept(
        traits::__is_wrapped_iterator_seekable_v<_Iterator_, _UnwrappedIterator_> == false || 
        traits::__is_wrapped_iterator_nothrow_seekable_v<_Iterator_, _UnwrappedIterator_>)
{
    if constexpr (traits::__is_wrapped_iterator_seekable_v<_Iterator_, _UnwrappedIterator_>)
        __iterator._Seek_to(std::forward<_UnwrappedIterator_>(__unwrapped_iterator));
    else __iterator = std::forward<_UnwrappedIterator_>(__unwrapped_iterator);
}
__RAZE_ALGORITHM_NAMESPACE_END
