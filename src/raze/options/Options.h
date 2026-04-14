#pragma once 

#include <src/raze/options/Option.h>
#include <src/raze/options/Settings.h>
#include <src/raze/options/Merge.h>
#include <src/raze/options/Drop.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::settings _Settings_ = settings<>>
struct options: 
    _Settings_
{
    constexpr inline options() noexcept: 
        _Settings_{} 
    {}

    template <concepts::option ... _Options_>
    constexpr inline explicit options(_Options_ && ... __options) noexcept:
        _Settings_(std::forward<_Options_>(__options) ...) 
    {}

    template <class ... _Options_>
    constexpr inline explicit options(const settings<_Options_...>& __options) noexcept :
        _Settings_(__options) 
    {}

    template <concepts::keyword _Keyword_>
    constexpr inline auto drop(const _Keyword_& __keyword) const noexcept {
        auto __dropped = raze::options::drop(__keyword, *this);
        return options<decltype(__dropped)>{__dropped};
    }

    template <
        concepts::keyword      _Keyword0_, 
        concepts::keyword ...  _Keywords_>
    constexpr inline auto drop(
        const _Keyword0_&       __keyword0,
        const _Keywords_& ...   __keywords) const noexcept 
    {
        auto __dropped = raze::options::drop(__keyword0, *this);
        return options<decltype(__dropped)>{__dropped}.drop(__keywords...);
    }

    template <concepts::keyword _Keywords_>
    constexpr inline auto extract(const _Keywords_& __keywords) const noexcept {
        auto __value = (*this)[__keywords];
        auto __dropped = raze::options::drop(__keywords, *this);

        return std::tuple{__value, options<decltype(__dropped)>{__dropped}};
    }
};

template <concepts::option ... _Options_>
options(_Options_&& ... __options) -> 
    options<decltype(settings(std::forward<_Options_>(__options) ...))>;

template <class ... _Options_>
options(const settings<_Options_...>&) ->
    options<settings<_Options_...>>;

template <
    concepts::settings _S0_, 
    concepts::settings _S1_>
constexpr static inline auto merge_prefer_first(
    const options<_S0_>& __base, 
    const options<_S1_>& __new_options) noexcept
{
    auto __result_options = raze::options::merge(__new_options, __base);
    return options<decltype(__result_options)>{__result_options};
}

template <class _Type_>
concept callable_options = concepts::settings<_Type_>;

template <class _Type_>
concept callable_option = concepts::option<_Type_>;

template<auto Decorator> 
struct exact_option {
    constexpr auto process(
        auto const&                                 __base, 
        concepts::exactly<Decorator> auto const&    __options) const noexcept
    {
        return raze::options::merge_prefer_first(__base, options{ __options });
    }

     constexpr auto default_to(auto const& __base) const { 
         return __base;
     }
};

__RAZE_OPTIONS_NAMESPACE_END
