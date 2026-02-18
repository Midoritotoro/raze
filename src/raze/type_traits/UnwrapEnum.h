#pragma once 

#include <type_traits>
#include <raze/RazeNamespace.h>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
	class	_Element_,
	bool	_IsEnum_ = std::is_enum_v<_Element_>>
struct __unwrap_enum {
	using type = std::underlying_type_t<_Element_>;
};

template <class _Element_>
struct __unwrap_enum<_Element_, false> {
	using type = _Element_;
};

template <class _Element_>
using unwrap_enum_type = typename __unwrap_enum<_Element_>::type;

__RAZE_TYPE_TRAITS_NAMESPACE_END
