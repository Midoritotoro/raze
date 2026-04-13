#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/TypeOr.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Keyword_>
struct as_keyword {
    using tag_type = _Keyword_;
    inline constexpr auto operator<=>(const as_keyword&) const noexcept = default;

    template <class _Type_>
    static constexpr bool accept() {
        if constexpr(concepts::same_as<std::remove_cvref_t<_Type_>, _Keyword_>) 
            return true;
        else if constexpr(concepts::__checks_for<_Keyword_, _Type_>)
            return _Keyword_::template check<_Type_>();
        else 
            return true;
    }

    template <class _Type_>
    constexpr auto operator=(_Type_&& __value) const noexcept
        requires(accept<_Type_>())
    {
        return option<_Keyword_, _Type_>{std::forward<_Type_>(__value)};
    }

    template <class _Type_>
    constexpr auto operator|(_Type_&& __value) const noexcept
        requires(accept<_Type_>()) 
    {
        return __type_or<_Keyword_, std::remove_cvref_t<_Type_>>{std::forward<_Type_>(__value)};
    }

    template <class _Function_> 
    constexpr auto operator|(call<_Function_>&& __callable) const noexcept
    {
        return __type_or<_Keyword_, call<_Function_>>{std::forward<_Function_>(__callable)};
    }

    template <concepts::option ... _Options_>
    constexpr decltype(auto) operator()(_Options_&& ... __options) const { 
        return fetch(_Keyword_{}, std::forward<_Options_>(__options)...); 
    }
};

template <
    class                   _ID_, 
    template <class> class  _Checker_>
struct checked_keyword:
    as_keyword<checked_keyword<_ID_, _Checker_>> 
{
    using as_keyword<checked_keyword<_ID_, _Checker_>>::operator=;

    template <class _Type_> 
    static constexpr bool check() { 
        return _Checker_<_Type_>::value; 
    }
};

template <
    class _ID_, 
    class _Type_>
struct typed_keyword: 
    as_keyword<typed_keyword<_ID_, _Type_>>
{
    using as_keyword<typed_keyword<_ID_, _Type_>>::operator=;

    template <class _T_>
    static constexpr bool check() { 
        return std::is_same_v<std::remove_cvref_t<_T_>, _Type_>; 
    }
};
    
template <class _ID_>
struct any_keyword: 
    as_keyword<any_keyword<_ID_>>
{
    using as_keyword<any_keyword<_ID_>>::operator=;
    using id_type = _ID_;
};
    
template <class _ID_> 
struct flag_keyword {
constexpr flag_keyword() {}
    constexpr flag_keyword(const _ID_&) {}
    using id_type = _ID_;

    template<class _Type_>
    static constexpr bool accept() {
        return std::is_same_v<std::true_type, _Type_>;
    }

    using tag_type          = _ID_;
    using keyword_type      = flag_keyword;
    using stored_value_type = bool;
        
    template <class _Type_>
    constexpr auto operator=(_Type_&&) const noexcept { 
        return *this; 
    }

    template <class _Type_>
    constexpr auto operator|(_Type_&& __value) const noexcept {
        return _::__type_or<flag_keyword, std::remove_cvref_t<_Type_>>{std::forward<_Type_>(__value)};
    }

    template <class _Function_> 
    constexpr auto operator|(call<_Function_>&& __callable) const noexcept {
        return _::__type_or<flag_keyword, call<_Function_>>{std::forward<_Function_>(__callable)};
    }

    constexpr auto operator()(const keyword_type&) const noexcept { 
        return true; 
    }

    template <
        class       _Option0_,
        class       _Option1_,
        class ...   _Options_>
    constexpr decltype(auto) operator()(_Option0_&&, _Option1_&&, _Options_&& ...) const {
        return  stdfix::same_as<keyword_type, typename std::remove_cvref_t<_Option0_>::keyword_type>
            ||  stdfix::same_as<keyword_type, typename std::remove_cvref_t<_Option1_>::keyword_type>
            || (stdfix::same_as<keyword_type, typename std::remove_cvref_t<_Options_>::keyword_type> || ...);
    }
};

template <class _Tag_>
constexpr flag_keyword<_Tag_> flag(_Tag_ __id) noexcept { 
    return {}; 
}

template <class _ID_>
constexpr any_keyword<_ID_> keyword(_ID_ __id) noexcept { 
    return {}; 
}

template <
    template <class> class  _Checker_,
    class                   _ID_>
constexpr checked_keyword<_ID_, _Checker_> keyword(_ID_ __id) noexcept { 
    return {}; 
}

template <
    class _Type_,
    class _ID_>
constexpr typed_keyword<_ID_, _Type_> keyword(_ID_ __id) noexcept { 
    return {};
}

template<typename... T> struct types {};


template <
    class                     _Settings_, 
    template <class...> class _List_ = types> 
struct __keywords;

template <
    class                       _Settings_, 
    template <class...> class   _List_ = types> 
struct __values;
        
template <
    class ...                   _Options_, 
    template <class...> class   _List_>
struct __keywords<settings<_Options_...>, _List_> {
    using type = _List_<typename _Options_::keyword_type...>;
};

template <
    class ...                   _Options_, 
    template <class...> class   _List_>
struct __values<settings<_Options_...>, _List_> {
    using type = _List_<typename _Options_::stored_value_type...>;
};

template <
    class                       _Settings_, 
    template <class...> class   _List_ = types>
using __keywords_t = typename __keywords<_Settings_, _List_>::type;

template <
    class                       _Settings_, 
    template <class...> class   _List_ = types>
using __values_t = typename __values<_Settings_, _List_>::type;


template <
    template <class...> class   _List_, 
    class ...                   _Options_>
constexpr auto keywords(const settings<_Options_...>&) noexcept {
    return __keywords_t<settings<_Options_...>, _List_>{
        typename _Options_::__keyword_type{}...};
}

template <
    template <class...> class   _List_, 
    class ...                   _Options_>
constexpr auto values(const settings<_Options_...>& __settings) noexcept {
    return __values_t<settings<_Options_...>, _List_>{
        __settings[typename _Options_::keyword_type{}]... };
}

__RAZE_OPTIONS_NAMESPACE_END
