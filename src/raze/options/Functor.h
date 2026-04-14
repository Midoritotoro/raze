#pragma once 


#include <src/raze/options/Options.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <template <class> class _Function_>
constexpr inline auto functor = _Function_<raze::options::options<>>{};

__RAZE_OPTIONS_NAMESPACE_END
