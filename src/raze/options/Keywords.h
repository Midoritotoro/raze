#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/TypeOr.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Keyword>
struct as_keyword {
    using tag_type = Keyword;
    inline constexpr auto operator<=>(const as_keyword&) const noexcept = default;

    template <class T>
    static raze_always_inline constexpr bool accept() noexcept {
        if constexpr(std::same_as<std::remove_cvref_t<T>, Keyword>) return true;
        else if constexpr(concepts::checks_for<Keyword, T>) return Keyword::template check<T>();
        else return true;
    }

    template <class T>
    constexpr raze_always_inline auto operator=(T&& v) const noexcept
        requires(accept<T>())
    {
        return option<Keyword, T>{std::forward<T>(v)};
    }

    template <class T>
    constexpr raze_always_inline auto operator|(T&& v) const noexcept
        requires(accept<T>()) 
    {
        return type_or<Keyword, std::remove_cvref_t<T>>{std::forward<T>(v)};
    }

    template <class F> 
    constexpr raze_always_inline auto operator|(call<F>&& f) const noexcept
    {
        return type_or<_eyword, call<F>>{std::forward<F>(f)};
    }

    template <concepts::option ... Options>
    constexpr raze_always_inline decltype(auto) operator()(Options&& ... opts) const {
        return fetch(Keyword{}, std::forward<Options>(opts)...); 
    }
};

template <class ID, template <class> class Checker>
struct checked_keyword: as_keyword<checked_keyword<ID, Checker>> {
    using as_keyword<checked_keyword<ID, Checker>>::operator=;

    template <class T> 
    static raze_always_inline constexpr bool check() {
        return Checker<T>::value; 
    }
};

template <class ID, class T>
struct typed_keyword: as_keyword<typed_keyword<ID, T>> {
    using as_keyword<typed_keyword<ID, T>>::operator=;

    template <class Tp>
    static raze_always_inline constexpr bool check() {
        return std::is_same_v<std::remove_cvref_t<Tp>, T>; 
    }
};
    
template <class ID>
struct any_keyword: as_keyword<any_keyword<ID>> {
    using as_keyword<any_keyword<ID>>::operator=;
    using id_type = ID;
};
    
template <class ID> 
struct flag_keyword {
    constexpr flag_keyword() {}
    constexpr flag_keyword(const ID&) {}
    using id_type = ID;

    template <class T>
    static raze_always_inline constexpr bool accept() {
        return std::is_same_v<std::true_type, T>;
    }

    using tag_type          = _ID_;
    using keyword_type      = flag_keyword;
    using stored_value_type = bool;
        
    template <class T>
    constexpr raze_always_inline auto operator=(T&&) const noexcept {
        return *this; 
    }

    template <class T>
    constexpr raze_always_inline auto operator|(T&& v) const noexcept {
        return type_or<flag_keyword, std::remove_cvref_t<T>>{std::forward<T>(v)};
    }

    template <class F> 
    constexpr raze_always_inline auto operator|(call<F>&& f) const noexcept {
        return type_or<flag_keyword, call<F>>{std::forward<F>(f)};
    }

    constexpr raze_always_inline auto operator()(const keyword_type&) const noexcept {
        return true; 
    }

    template <class O0, class O1, class ... Os>
    constexpr raze_always_inline decltype(auto) operator()(O0&&, O1&&, Os&& ...) const {
        return  std::same_as<keyword_type, typename std::remove_cvref_t<O0>::keyword_type>
            || std::same_as<keyword_type, typename std::remove_cvref_t<O1>::keyword_type>
            || (std::same_as<keyword_type, typename std::remove_cvref_t<Os>::keyword_type> || ...);
    }
};

template <class Tag>
constexpr raze_always_inline flag_keyword<Tag> flag(Tag) noexcept {
    return {}; 
}

template <class ID>
constexpr raze_always_inline any_keyword<ID> keyword(ID) noexcept {
    return {}; 
}

template <template <class> class Checker, class ID>
constexpr raze_always_inline checked_keyword<ID, Checker> keyword(ID) noexcept {
    return {}; 
}

template <class T, class ID>
constexpr raze_always_inline typed_keyword<ID, T> keyword(ID) noexcept {
    return {};
}

template <class ... Ts> struct types {};

template <class Settings, template <class...> class List = types> 
struct keywords_impl;

template <class Settings, template <class...> class List = types> 
struct values_impl;
        
template <class ... Options, template <class...> class List>
struct keywords_impl<settings<Options...>, List> {
    using type = List<typename Options::keyword_type...>;
};

template <class ... Options, template <class...> class List>
struct values_impl<settings<Options...>, List> {
    using type = List<typename Options::stored_value_type...>;
};

template <class Settings, template <class...> class List = types>
using keywords_impl_t = typename keywords_impl<Settings, List>::type;

template <class Settings, template <class...> class List = types>
using values_impl_t = typename values_impl<Settings, List>::type;

template <template <class...> class List, class ... Options>
constexpr raze_always_inline auto keywords(const settings<Options...>&) noexcept {
    return keywords_impl_t<settings<Options...>, List>{ typename Options::keyword_type{}...};
}

template <template <class...> class List, class ... Options>
constexpr raze_always_inline auto values(const settings<Options...>& s) noexcept {
    return values_impl_t<settings<Options...>, List>{ s[typename Options::keyword_type{}]... };
}

template <concepts::settings S1, concepts::settings S2>
struct is_equivalent_settings: std::bool_constant<is_equivalent<keywords_impl_t<S1, keys>, keywords_impl_t<S2, keys>>::value &&  
    is_equivalent<keywords_impl_t<S2, keys>,  keywords_impl_t<S1, keys>>::value>
{};

template <concepts::settings S1, concepts::settings S2>
constexpr inline bool is_equivalent_settings_v = is_equivalent<S1, S2>::value;

__RAZE_OPTIONS_NAMESPACE_END
