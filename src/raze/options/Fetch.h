#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/TypeOr.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    concepts::keyword       _Keyword_, 
    concepts::option ...    _Options_>
constexpr raze_always_inline decltype(auto) fetch(
    const _Keyword_&        __keyword,
    const _Options_& ...    __options) noexcept
{
    const auto __opts = settings(__options...);
    return __opts[__keyword];
}
    
template <
    concepts::keyword       _Keyword_, 
    class                   _Value_,
    concepts::option ...    _Options_>
constexpr raze_always_inline decltype(auto) fetch(
    const __type_or<_Keyword_, _Value_>&    __keyword,
    const _Options_& ...                    __options) noexcept
{
    const auto __opts = settings(__options...);
    return __opts[__keyword];
}

template <
    class               _Key_, 
    concepts::settings  _Settings_>
constexpr raze_always_inline decltype(auto) fetch(
    const _Key_&        __key,
    const _Settings_&   __settings) noexcept
{
    return __settings[__key];
}

template <
    auto        _Keyword_, 
    class ...   _Sources_> 
struct __fetch_t;

template <
    auto                    _Keyword_, 
    concepts::option ...    _Options_>
struct __fetch_t<_Keyword_, _Options_...>  {
    using type = decltype(fetch(_Keyword_, std::declval<_Options_>()...));
};

template <
    auto               _Keyword_,
    concepts::settings _Settings_>
struct __fetch_t<_Keyword_, _Settings_>  {
    using type = decltype(fetch(_Keyword_, std::declval<_Settings_>()));
};

template <
    auto        _Keyword_, 
    class ...   _Sources_>
using fetch_t = typename __fetch_t<_Keyword_, _Sources_...>::type;

__RAZE_OPTIONS_NAMESPACE_END
