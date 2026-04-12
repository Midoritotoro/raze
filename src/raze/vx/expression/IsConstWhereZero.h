#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Const_where_zero;

template <
	class _Where_,
	class = void>
struct __is_const_where_zero :
	std::false_type
{};

template <class _ConstWhereZero_>
struct __is_const_where_zero<
    _ConstWhereZero_,
    std::void_t<
            _Const_where_zero<
                typename _ConstWhereZero_::datapar_type,
                typename _ConstWhereZero_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Const_where_zero<
                typename _ConstWhereZero_::datapar_type,
                typename _ConstWhereZero_::mask_type>,
    _ConstWhereZero_> ||
        std::is_same_v<
            _Const_where_zero<
                typename _ConstWhereZero_::datapar_type,
				typename _ConstWhereZero_::mask_type>,
            _ConstWhereZero_>>
{};

template <class _ConstWhereZero_>
constexpr bool __is_const_where_zero_v = __is_const_where_zero<std::remove_cvref_t<_ConstWhereZero_>>::value;

__RAZE_VX_NAMESPACE_END
