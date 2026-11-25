#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::option ... Options>
struct settings;

template <concepts::option ... Keys1, concepts::option ... Keys2>
constexpr raze_always_inline auto merge(const settings<Keys1...>& opts, 
    const settings<Keys2...>& defaults) noexcept
{
    if constexpr (sizeof...(Keys2) == 0) {
        return opts;
    }
    else if constexpr (sizeof...(Keys1) == 0) {
        return defaults;
    }
    else {
        auto selector = [] <class Key, class Options> (
            const Key&, const Options& os, const auto& d) raze_always_inline_lambda
        {
            constexpr Key key;
            if constexpr(Options::contains(key)) return (key = os[key]);
            else return (key = d[key]);
        };

        auto select = [&] <class ... Keys> (const keys<Keys...>&, const auto& os, const auto& ds) raze_always_inline_lambda {
            return settings(selector(Keys{}, os, ds)...);
        };

        return select(typename uniques<keys<typename Keys1::keyword_type...>,
            keys<typename Keys2::keyword_type...>>::type{}, opts, defaults);
    }
}

__RAZE_OPTIONS_NAMESPACE_END
