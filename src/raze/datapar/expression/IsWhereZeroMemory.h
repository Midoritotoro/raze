#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN


template <
    class _IteratorType_,
    class _MaskType_>
class _Where_zero_memory;

template <
	class _WhereZeroMemory_,
	class = void>
struct __is_where_zero_memory:
	std::false_type
{};

template <class _WhereZeroMemory_>
struct __is_where_zero_memory<
    _WhereZeroMemory_,
    std::void_t<
        _Where_zero_memory<
            typename _WhereZeroMemory_::iterator_type,
            typename _WhereZeroMemory_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Where_zero_memory<
                typename _WhereZeroMemory_::iterator_type,
                typename _WhereZeroMemory_::mask_type>,
    _WhereZeroMemory_> ||
        std::is_same_v<
            _Where_zero_memory<
                typename _WhereZeroMemory_::iterator_type,
				typename _WhereZeroMemory_::mask_type>,
            _WhereZeroMemory_>> 
{};

template <class _WhereZeroMemory_>
constexpr bool __is_where_zero_memory_v = __is_where_zero_memory<std::remove_cvref_t<_WhereZeroMemory_>>::value;

__RAZE_DATAPAR_NAMESPACE_END
