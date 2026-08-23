#pragma once 

#include <src/raze/options/Options.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <template <class> class F>
constexpr inline auto functor = F<raze::options::options<>>{};

__RAZE_OPTIONS_NAMESPACE_END
