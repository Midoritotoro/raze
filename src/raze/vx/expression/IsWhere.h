#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN


template <
    class _DataparType_,
    class _MaskType_>
class _Where;

template <
	class _Where_,
	class = void>
struct __is_where :
	std::false_type
{};

template <class _Where_>
struct __is_where<
    _Where_,
    std::void_t<
            _Where<
                typename _Where_::datapar_type,
                typename _Where_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Where<
                typename _Where_::datapar_type,
                typename _Where_::mask_type>,
    _Where_> ||
        std::is_same_v<
            _Where<
                typename _Where_::datapar_type,
				typename _Where_::mask_type>,
            _Where_>> 
{};

template <class _Where_>
constexpr bool __is_where_v = __is_where<std::remove_cvref_t<_Where_>>::value;

__RAZE_VX_NAMESPACE_END
