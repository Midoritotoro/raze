#pragma once 

#include <src/raze/options/SelectKeys.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    concepts::keyword       _Keyword_,
    concepts::option ...    _Options_>
constexpr auto drop(
    const _Keyword_&                __keyword, 
    const settings<_Options_...>&   __settings) noexcept
{
    using selected_keys_t = typename select_keys<_Keyword_, settings<_Options_...>>::type;
        
    return [&]<class ... _Keys_>(keys<_Keys_...>) {
        return settings { (_Keys_{} = __settings[_Keys_{}])...};
    }(selected_keys_t{});
}

__RAZE_OPTIONS_NAMESPACE_END
