#pragma once 

#include <src/raze/type_traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN


template <
    class _DataparType_,
    class _MaskType_>
class _Const_where;

template <
	class _Where_,
	class = void>
struct __is_const_where :
	std::false_type
{};

template <class _ConstWhere_>
struct __is_const_where<
    _ConstWhere_,
    std::void_t<
            _Const_where<
                typename _ConstWhere_::datapar_type,
                typename _ConstWhere_::mask_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Const_where<
                typename _ConstWhere_::datapar_type,
                typename _ConstWhere_::mask_type>,
    _ConstWhere_> ||
        std::is_same_v<
            _Const_where<
                typename _ConstWhere_::datapar_type,
				typename _ConstWhere_::mask_type>,
            _ConstWhere_>>
{};

template <class _ConstWhere_>
constexpr bool __is_const_where_v = __is_const_where<std::remove_cvref_t<_ConstWhere_>>::value;

__RAZE_VX_NAMESPACE_END
