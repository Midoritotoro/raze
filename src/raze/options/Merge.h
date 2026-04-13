#pragma once 

#include <raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    concepts::option ... _Keys1_, 
    concepts::option ... _Keys2_>
constexpr auto merge(
    const settings<_Keys1_...>& __options, 
    const settings<_Keys2_...>& __definition) noexcept
{
    auto __selector = [] <class _Key_, class _Options_> (
        const _Key_&, 
        const _Options_&    __opts, 
        const auto&         __d)
    {
        constexpr _Key_ __key;

        if constexpr(_Options_::contains(__key)) 
            return (__key = __opts[__key]);
        else 
            return (__key = __d[__key]);
    };

    auto __select = [&] <class ... _Keys_> (
        const keys<_Keys_...>&, 
        const auto& __os, 
        const auto& __ds)  
    {
        return settings(__selector(_Keys_{}, __os, __ds)...);
    };

    return __select(typename uniques<keys<typename _Keys1_::keyword_type...>,
        keys<typename _Keys2_::keyword_type...>>::type{}, __options, __definition);
}

__RAZE_OPTIONS_NAMESPACE_END
