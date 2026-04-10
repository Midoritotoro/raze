#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN


template <
    class _DataparType_,
    class _MaskType_>
class _Where_zero;

template <
	class _WhereZero_,
	class = void>
struct __is_where_zero:
	std::false_type
{};

template <class _WhereZero_>
struct __is_where_zero<
    _WhereZero_,
    std::void_t<
        _Where_zero<
                typename _WhereZero_::datapar_type,
                typename _WhereZero_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Where_zero<
                typename _WhereZero_::datapar_type,
                typename _WhereZero_::mask_type>,
    _WhereZero_> ||
        std::is_same_v<
            _Where_zero<
                typename _WhereZero_::datapar_type,
				typename _WhereZero_::mask_type>,
            _WhereZero_>> 
{};

template <class _WhereZero_>
constexpr bool __is_where_zero_v = __is_where_zero<std::remove_cvref_t<_WhereZero_>>::value;

__RAZE_VX_NAMESPACE_END
