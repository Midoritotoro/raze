#pragma once 

#include <src/raze/options/Aggregator.h>
#include <src/raze/options/Keywords.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::option ... _Options_> 
struct settings {
    using settings_type = void;
    using base = aggregator<_Options_...>;

    constexpr settings(const _Options_& ... __options) noexcept: 
        _content(__options...) 
    {}

    static constexpr std::ptrdiff_t size() noexcept { 
        return sizeof...(_Options_); 
    }

    template <concepts::keyword _Key_>
    static constexpr auto contains(const _Key_& __keyword) noexcept {
        using found = decltype((std::declval<base>())(_Key_{}));
        return !concepts::same_as<found, unknown_key>;
    }
        
    template <concepts::keyword ... _Keys_>
    static constexpr auto contains_any(_Keys_ ... __keys) noexcept { 
        return (contains(__keys) || ...); 
    }

    template <concepts::keyword ... _Keys_>
    static constexpr auto contains_only(const _Keys_& ... __keys) noexcept {
        using current_keys = keys<typename _Options_::keyword_type...>;
        using acceptable_keys = keys<_Keys_...>;
        using unique_set = typename uniques<current_keys, acceptable_keys>::type;

        return is_equivalent<unique_set, acceptable_keys>::value;
    }

    template <concepts::keyword... _Keys_>
    static constexpr auto contains_none(_Keys_ ... __keys) noexcept {
        return !contains_any(__keys...); 
    }

    template <concepts::keyword _Key_> 
    constexpr auto operator[](const _Key_& __key) const noexcept {
        return _content(__key);
    }

    template <class _Keyword_>
    constexpr auto operator[](const flag_keyword<_Keyword_>&) const noexcept {
        return contains(flag_keyword<_Keyword_>{});
    }

    template <
        concepts::keyword   _Key_, 
        class               _Value_>
    constexpr auto operator[](__type_or<_Key_, _Value_> __value) const {
        if constexpr(contains(_Key_{})) 
            return (*this)[_Key_{}];
        else if constexpr(requires(_Value_ __t) { __t.perform(); })  
            return __value.value.perform();
        else 
            return __value.value;
    }
        
    base _content;
};

template <concepts::option ... _Options_>
settings(const _Options_& ... __options) -> settings<_Options_...>;

__RAZE_OPTIONS_NAMESPACE_END
