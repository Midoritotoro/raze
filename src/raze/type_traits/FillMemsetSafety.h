#pragma once 

#include <src/raze/type_traits/TypeTraits.h>
#include <raze/compatibility/FunctionAttributes.h>

#include <src/raze/type_traits/IntegralProperties.h>

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
    class _ForwardIterator_,
    class _Type_,
    bool = is_iterator_contiguous_v<_ForwardIterator_>>
constexpr bool __is_fill_memset_safe_v = std::conjunction_v<
    std::is_scalar<_Type_>,
    is_character_or_byte_or_bool<
        unwrap_enum_type<
            std::remove_reference_t<
                iterator_reference_type<_ForwardIterator_>>>>,
    std::negation<
        std::is_volatile<
            std::remove_reference_t<
                iterator_reference_type<_ForwardIterator_>>>>,
    std::is_assignable<
        iterator_reference_type<_ForwardIterator_>,
    const _Type_ &>>;

template <
    class _ForwardIterator_,
    class _Type_>
constexpr bool __is_fill_memset_safe_v<_ForwardIterator_, _Type_, false> = false;

template <
    class _ForwardIterator_,
    class _Type_,
    bool = is_iterator_contiguous_v<_ForwardIterator_>>
constexpr bool __is_fill_zero_memset_safe_v =
    std::conjunction_v<
        std::is_scalar<_Type_>, 
        std::is_scalar<
            iterator_value_type<_ForwardIterator_>>,
        std::negation<
            std::is_member_pointer<
                iterator_value_type<_ForwardIterator_>>>,
        std::negation<
            std::is_volatile<
                std::remove_reference_t<
                    iterator_reference_type<_ForwardIterator_>>>>,
        std::is_assignable<iterator_reference_type<_ForwardIterator_>,
    const _Type_&>>;

template <
    class _ForwardIterator_,
    class _Type_>
constexpr bool __is_fill_zero_memset_safe_v<_ForwardIterator_, _Type_, false> = false;

__RAZE_TYPE_TRAITS_NAMESPACE_END
