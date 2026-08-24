#pragma once 

#include <raze/RazeNamespace.h>

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <type_traits>
#include <xmemory>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class	T, class ... Ts>
constexpr inline bool is_any_of_v = (std::same_as<T, Ts> || ...);

__RAZE_TRAITS_NAMESPACE_END
