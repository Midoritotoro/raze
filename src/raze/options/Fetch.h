#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/TypeOr.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::keyword Keyword, concepts::option ... Options>
constexpr raze_always_inline decltype(auto) fetch(const Keyword& kw,
    const Options& ... opts) noexcept
{
    return settings(opts...)[kw];
}
    
template <concepts::keyword Keyword, class Value, concepts::option ... Options>
constexpr raze_always_inline decltype(auto) fetch(const type_or<Keyword, Value>& kw,
    const Options& ... opts) noexcept
{
    return settings(opts...)[kw];
}

template <class Key, concepts::settings Settings>
constexpr raze_always_inline decltype(auto) fetch(const Key& key,
    const Settings& s) noexcept
{
    return s[key];
}

template <auto Keyword, class ... Sources> 
struct fetch_t_impl;

template <auto Keyword, concepts::option ... Options>
struct fetch_t_impl<Keyword, Options...>  {
    using type = decltype(fetch(Keyword, std::declval<Options>()...));
};

template <auto Keyword, concepts::settings Settings>
struct fetch_t_impl<Keyword, Settings>  {
    using type = decltype(fetch(Keyword, std::declval<Settings>()));
};

template <auto Keyword, class ... Sources>
using fetch_t = typename fetch_t_impl<Keyword, Sources...>::type;

__RAZE_OPTIONS_NAMESPACE_END
