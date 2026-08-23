#pragma once 

#include <src/raze/options/SelectKeys.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::keyword Keyword, concepts::option ... Options>
constexpr raze_always_inline auto drop(const Keyword&, 
    const settings<Options...>& s) noexcept
{
    using selected_keys_t = typename select_keys<Keyword, settings<Options...>>::type;
        
    return [&] <class ... Keys> (keys<Keys...>) {
        return settings { (Keys{} = s[Keys{}])...};
    }(selected_keys_t{});
}

__RAZE_OPTIONS_NAMESPACE_END
