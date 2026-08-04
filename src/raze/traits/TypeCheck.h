#pragma once 

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <type_traits>
#include <xmemory>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class	_Type_, class ... _Types_>
constexpr inline bool is_any_of_v = (std::is_same_v<_Type_, _Types_> || ...);

__RAZE_TRAITS_NAMESPACE_END
