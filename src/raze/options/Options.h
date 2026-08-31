#pragma once 

#include <src/raze/options/Option.h>
#include <src/raze/options/Settings.h>
#include <src/raze/options/Merge.h>
#include <src/raze/options/Drop.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::settings Settings = settings<>>
struct options: 
    Settings
{
    constexpr raze_always_inline options() noexcept:
        Settings{} 
    {}

    template <concepts::option ... Options>
    constexpr raze_always_inline explicit options(Options&& ... opts) noexcept:
        Settings(std::forward<Options>(opts)...) 
    {}

    template <class ... Options>
    constexpr raze_always_inline explicit options(const settings<Options...>& opts) noexcept :
        Settings(opts) 
    {}

    template <concepts::keyword Keyword>
    constexpr raze_always_inline auto drop(const Keyword& kw) const noexcept {
        auto dropped = raze::options::drop(kw, *this);
        return options<decltype(dropped)>{dropped};
    }

    template <concepts::keyword Keyword0, concepts::keyword ... Keywords>
    constexpr raze_always_inline auto drop(const Keyword0& kw0,
        const Keywords& ... kws) const noexcept
    {
        auto dropped = raze::options::drop(kw0, *this);
        return options<decltype(dropped)>{dropped}.drop(kws...);
    }

    template <concepts::keyword Keyword>
    constexpr raze_always_inline auto extract(const Keyword& kw) const noexcept {
        auto value = (*this)[kw];
        auto dropped = raze::options::drop(kw, *this);

        return std::tuple{value, options<decltype(dropped)>{dropped}};
    }
};

template <concepts::option ... Options>
options(Options&& ... opts) -> 
    options<decltype(settings(std::forward<Options>(opts)...))>;

template <class ... Options>
options(const settings<Options...>&) ->
    options<settings<Options...>>;

template <concepts::settings S0, concepts::settings S1>
constexpr static raze_always_inline auto merge_prefer_first(
    const options<S0>& base, const options<S1>& new_options) noexcept
{
    auto result_options = raze::options::merge(new_options, base);
    return options<decltype(result_options)>{result_options};
}

template <class T>
concept callable_options = concepts::settings<T>;

template <class T>
concept callable_option = concepts::option<T>;

template <auto Decorator> 
struct exact_option {
    constexpr raze_always_inline auto process(auto const& base, 
        concepts::exactly<Decorator> auto const& opts) const noexcept
    {
        return merge_prefer_first(base, options{ opts });
    }

     constexpr raze_always_inline auto default_to(auto const& base) const {
         return base;
     }
};

__RAZE_OPTIONS_NAMESPACE_END
