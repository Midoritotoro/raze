#pragma once 

#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/IteratorCheck.h>

#include <src/raze/type_traits/UnwrapEnum.h>
#include <algorithm>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
    class _Source_,
    class _Destination_,
    class _SourceReference_,
    class _DestinationReference_>
struct __trivial_iterator_category {
    using UnwrappedSource       = unwrap_enum_type<_Source_>;
    using UnwrappedDestination  = unwrap_enum_type<_Destination_>;

    static constexpr bool SameSizeAndCompatible =
        sizeof(_Source_) == sizeof(_Destination_)
        // If UnwrappedDestination is bool, UnwrappedSource also needs to be bool
        // Conversion from non-bool => non-bool | bool => bool | bool => non-bool is fine.
        // Conversion from non-bool => bool is not fine.
        && std::is_same_v<bool, UnwrappedSource> 
            >= std::is_same_v<bool, UnwrappedDestination>
        && (std::is_same_v<UnwrappedSource, UnwrappedDestination> 
        || (std::is_integral_v<UnwrappedSource> 
                && std::is_integral_v<UnwrappedDestination>)
        || (std::is_floating_point_v<UnwrappedSource> 
            && std::is_floating_point_v<UnwrappedDestination>));

    static constexpr bool BitcopyConstructible =
        SameSizeAndCompatible && std::is_trivially_constructible_v<_Destination_, _SourceReference_>;

    static constexpr bool BitcopyAssignable =
        SameSizeAndCompatible && std::is_trivially_assignable_v<_DestinationReference_, _SourceReference_>;
};

template <
    class _Source_,
    class _Destination_, 
    class _SourceReference_,
    class _DestinationReference_>
struct __trivial_iterator_category<
    _Source_*, _Destination_*,
    _SourceReference_, _DestinationReference_>
{
    static constexpr bool BitcopyConstructible =
        is_pointer_address_convertible_v<_Source_, _Destination_> 
        && std::is_trivially_constructible_v<_Destination_*, _SourceReference_>;

    static constexpr bool BitcopyAssignable =
        is_pointer_address_convertible_v<_Source_, _Destination_>
        && std::is_trivially_assignable_v<_DestinationReference_, _SourceReference_>;
};

struct __false_trivial_iterator_category {
    static constexpr bool BitcopyConstructible = false;
    static constexpr bool BitcopyAssignable = false;
};

template <
    class _SourceIterator_, 
    class _DestinationIterator_,
    bool AreContiguous = 
        is_iterator_contiguous_v<_SourceIterator_> 
            && is_iterator_contiguous_v<_DestinationIterator_>
            && !is_iterator_volatile_v<_SourceIterator_>
            && !is_iterator_volatile_v<_DestinationIterator_>>
struct __move_iterator_category : 
    __trivial_iterator_category<
        iterator_value_type<_SourceIterator_>, 
        iterator_value_type<_DestinationIterator_>,
std::remove_reference_t<
    iterator_reference_type<_SourceIterator_>>&&,
    iterator_reference_type<_DestinationIterator_>
> 
{};

template <
    class _SourceIterator_,
    class _DestinationIterator_>
struct __move_iterator_category<
    std::move_iterator<_SourceIterator_>, _DestinationIterator_, false> : 
        __move_iterator_category<_SourceIterator_, _DestinationIterator_> {
};

template <
    class _SourceIterator_,
    class _DestinationIterator_,
    bool AreContiguous = 
        is_iterator_contiguous_v<_SourceIterator_> 
            && is_iterator_contiguous_v<_DestinationIterator_>
            && !is_iterator_volatile_v<_SourceIterator_>
        && !is_iterator_volatile_v<_DestinationIterator_>>
struct __copy_iterator_category :
    __trivial_iterator_category <
        iterator_value_type<_SourceIterator_>,
        iterator_value_type<_DestinationIterator_>,
        iterator_reference_type<_SourceIterator_>,
        iterator_reference_type<_DestinationIterator_>
    > 
{};

template <
    class _SourceIterator_,
    class _DestinationIterator_>
struct __copy_iterator_category<
    _SourceIterator_, _DestinationIterator_, false>: 
        __false_trivial_iterator_category
{};

template <
    class _SourceIterator_,
    class _DestinationIterator_>
struct __copy_iterator_category<
    std::move_iterator<_SourceIterator_>, _DestinationIterator_, false>: 
        __move_iterator_category<_SourceIterator_, _DestinationIterator_>
{};

template <
    class _InputIterator_,
    class _Sentinel_,
    class _OutIterator_>
using __copy_sentinel_category = std::conditional_t<
#if base_has_cxx20
    std::is_same_v<
        _Sentinel_, _InputIterator_>
    || std::sized_sentinel_for<
        _Sentinel_, _InputIterator_>,
#else
    std::is_same_v<_Sentinel_, _InputIterator_>,
#endif
    __copy_iterator_category<
        _InputIterator_, _OutIterator_>,
    __false_trivial_iterator_category>;

__RAZE_TYPE_TRAITS_NAMESPACE_END
