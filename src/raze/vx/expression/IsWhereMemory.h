#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN


template <
    class _IteratorType_,
    class _DataparType_,
    class _MaskType_>
class _Where_memory;

template <
	class _WhereMemory_,
	class = void>
struct __is_where_memory:
	std::false_type
{};

template <class _WhereMemory_>
struct __is_where_memory<
    _WhereMemory_,
    std::void_t<
        _Where_memory<
            typename _WhereMemory_::iterator_type,
            typename _WhereMemory_::datapar_type,
            typename _WhereMemory_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Where_memory<
                typename _WhereMemory_::iterator_type,
                typename _WhereMemory_::datapar_type,
                typename _WhereMemory_::mask_type>,
    _WhereMemory_> ||
        std::is_same_v<
            _Where_memory<
                typename _WhereMemory_::iterator_type,
                typename _WhereMemory_::datapar_type,
				typename _WhereMemory_::mask_type>,
            _WhereMemory_>> 
{};

template <class _WhereMemory_>
constexpr bool __is_where_memory_v = __is_where_memory<std::remove_cvref_t<_WhereMemory_>>::value;

__RAZE_VX_NAMESPACE_END
