#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/RazeNamespace.h>

#include <raze/math/IntegralTypesConversions.h>
#include <xutility>

#include <src/raze/algorithm/AlgorithmDebug.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Iterator_>
raze_nodiscard constexpr decltype(auto) __unwrap_iterator(_Iterator_&& __iterator) 
    noexcept(type_traits::__is_iterator_unwrappable_v<_Iterator_> == false || type_traits::__is_nothrow_unwrappable_v<_Iterator_>)
{
    if constexpr (std::is_pointer_v<std::decay_t<_Iterator_>>)
        return __iterator + 0;
    else if constexpr (type_traits::__is_iterator_unwrappable_v<_Iterator_>)
        return std::move(__iterator)._Unwrapped();
    else
        return std::move(__iterator);
}

template <class _Iterator_>
using __unwrapped_iterator_type = std::remove_cvref_t<decltype(__unwrap_iterator(std::declval<_Iterator_>()))>;

template <class _Iterator_>
raze_nodiscard constexpr decltype(auto) __unwrap_unverified_iterator(_Iterator_&& __iterator) 
    noexcept(type_traits::__is_possibly_unverified_iterator_unwrappable_v<_Iterator_> == false)
{
    if constexpr (std::is_pointer_v<std::decay_t<_Iterator_>>)
        return __iterator + 0;
    else if constexpr (type_traits::__is_possibly_unverified_iterator_unwrappable_v<_Iterator_>)
        return std::move(__iterator)._Unwrapped();
    else
        return std::move(__iterator);
}

template <class _Iterator_>
using __unwrapped_unverified_iterator_type = std::remove_cvref_t<decltype(__unwrap_unverified_iterator(std::declval<_Iterator_>()))>;

template <
    class _Iterator_, 
    class _DifferenceType_>
raze_nodiscard constexpr decltype(auto) __unwrap_iterator_offset(
    _Iterator_&&            __iterator,
    const _DifferenceType_  __offset) noexcept(
        type_traits::__is_possibly_unverified_iterator_unwrappable_v<_Iterator_> == false ||
        (type_traits::__is_iterator_unwrappable_for_offset_v<_Iterator_> == false ||
            type_traits::__is_iterator_unwrappable_for_offset_v<_Iterator_>)
    )
{
    if constexpr (std::is_pointer_v<std::decay_t<_Iterator_>>) {
        return __iterator + 0;
    }
    else if constexpr (
        type_traits::__is_iterator_unwrappable_for_offset_v<_Iterator_> &&
        type_traits::is_nonbool_integral_v<_DifferenceType_>) 
    {
        using _IteratorDifferenceType      = type_traits::iterator_difference_type<std::remove_cvref_t<_Iterator_>>;
        using _CommonDifferenceType        = std::common_type_t<_DifferenceType_, _IteratorDifferenceType>;

        const auto __common_offset = static_cast<_CommonDifferenceType>(__offset);

        constexpr auto __maximum = math::__maximum_integral_limit<_IteratorDifferenceType>();
        constexpr auto __minimum = math::__minimum_integral_limit<_IteratorDifferenceType>();

        raze_debug_assert_log(__common_offset <= static_cast<_CommonDifferenceType>(__maximum)
            && (std::is_unsigned_v<_DifferenceType_> || static_cast<_CommonDifferenceType>(__minimum) <= __common_offset),
            "integer overflow");

        __iterator._Verify_offset(static_cast<_IteratorDifferenceType>(__offset));
        return std::move(__iterator)._Unwrapped();
    } 
    else if constexpr (type_traits::__is_possibly_unverified_iterator_unwrappable_v<_Iterator_>) {
        return std::move(__iterator)._Unwrapped();
    } 
    else {
        return std::move(__iterator);
    }
}

template <
    class _Iterator_,
    class _UnwrappedIterator_>
constexpr void __seek_possibly_wrapped_iterator(
    _Iterator_&             __iterator,
    _UnwrappedIterator_&&   __unwrapped_iterator) noexcept(
        type_traits::__is_wrapped_iterator_seekable_v<_Iterator_, _UnwrappedIterator_> == false || 
        type_traits::__is_wrapped_iterator_nothrow_seekable_v<_Iterator_, _UnwrappedIterator_>)
{
    if constexpr (type_traits::__is_wrapped_iterator_seekable_v<_Iterator_, _UnwrappedIterator_>)
        __iterator._Seek_to(std::forward<_UnwrappedIterator_>(__unwrapped_iterator));
    else
        __iterator = std::forward<_UnwrappedIterator_>(__unwrapped_iterator);
}

template <
    class _Iterator_, 
    class _DifferenceType_>
raze_nodiscard constexpr decltype(auto) __unwrap_iterator_bytes_offset(
    _Iterator_&&            __iterator,
    const _DifferenceType_  __offset_bytes) noexcept(
        type_traits::__is_possibly_unverified_iterator_unwrappable_v<_Iterator_> == false ||
        (type_traits::__is_iterator_unwrappable_for_offset_v<_Iterator_> == false ||
            type_traits::__is_iterator_unwrappable_for_offset_v<_Iterator_>))
{
    using _ValueType = type_traits::iterator_value_type<__unwrapped_iterator_type<_Iterator_>>;

    if constexpr (std::is_pointer_v<std::decay_t<_Iterator_>>)
        return __iterator + 0;
    else
        return __unwrap_iterator_offset(std::move(__iterator), (__offset_bytes / sizeof(_ValueType)));
}

__RAZE_ALGORITHM_NAMESPACE_END
