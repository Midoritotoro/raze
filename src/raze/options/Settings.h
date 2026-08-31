#pragma once 

#include <src/raze/options/Aggregator.h>
#include <src/raze/options/Keywords.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::option ... Options> 
struct settings {
    using settings_type = void;
    using base = aggregator<Options...>;

    constexpr settings(const Options& ... opts) noexcept: 
        _content(opts...) 
    {}

    static raze_always_inline constexpr std::ptrdiff_t size() noexcept {
        return sizeof...(Options); 
    }

    template <concepts::keyword Keyword>
    static raze_always_inline constexpr auto contains(const Keyword& kw) noexcept {
        using found = decltype((std::declval<base>())(Keyword{}));
        return !std::same_as<found, unknown_key>;
    }
        
    template <concepts::keyword ... Keywords>
    static raze_always_inline constexpr auto contains_any(Keywords ... kws) noexcept {
        return (contains(kws) || ...); 
    }

    template <concepts::keyword ... Keywords>
    static raze_always_inline constexpr auto contains_only(const Keywords& ... kws) noexcept {
        using current_keys = keys<typename Options::keyword_type...>;
        using acceptable_keys = keys<Keywords...>;
        using unique_set = typename uniques<current_keys, acceptable_keys>::type;

        return is_equivalent<unique_set, acceptable_keys>::value;
    }

    template <concepts::keyword ... Keywords>
    static raze_always_inline constexpr auto contains_none(Keywords ... kws) noexcept {
        return !contains_any(kws...); 
    }

    template <concepts::keyword Keyword> 
    constexpr raze_always_inline auto operator[](const Keyword& kw) const noexcept {
        return _content(kw);
    }

    template <concepts::keyword ... Keywords> 
    constexpr raze_always_inline auto operator[](const Keywords& ... kws) const noexcept {
        return (_content(kws), ...);
    }

    template <class Keyword>
    constexpr raze_always_inline auto operator[](const flag_keyword<Keyword>&) const noexcept {
        return contains(flag_keyword<Keyword>{});
    }

    template <concepts::keyword Keyword, class Value>
    constexpr raze_always_inline auto operator[](type_or<Keyword, Value> v) const {
        if constexpr(contains(Keyword{})) return (*this)[Keyword{}];
        else if constexpr(requires(Value t) { t.perform(); }) return v._value.perform();
        else return v._value;
    }
        
    raze_no_unique_address base _content;
};

template <concepts::option ... Options>
settings(const Options& ... opts) -> settings<Options...>;

__RAZE_OPTIONS_NAMESPACE_END
