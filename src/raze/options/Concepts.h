#pragma once 

#include <src/raze/options/Keys.h>
#include <src/raze/options/Option.h>
#include <src/raze/options/SameAs.h>
#include <src/raze/options/Keywords.h>
#include <concepts>


__RAZE_OPTIONS_CONCEPTS_NAMESPACE_BEGIN

template <class _Key_>
concept keyword = requires(_Key_ __key) {
    typename _Key_::tag_type;
    { _Key_::template accept<int>() } -> concepts::same_as<bool>;
};

template <class _Option_>
concept option = requires(const _Option_& __option) {
    { __option(typename std::remove_cvref_t<_Option_>::keyword_type{}) }
        -> concepts::same_as<typename std::remove_cvref_t<_Option_>::stored_value_type>;
};

template <class _Settings_> 
concept settings = requires(const _Settings_& __settings) {
    typename _Settings_::settings_type;
};

template <
    class _Option_,
    auto  _Keyword_>
concept exactly = concepts::same_as<typename _Option_::keyword_type, 
    std::remove_cvref_t<decltype(_Keyword_)>>;

template <
    class _Key_,
    class _Type_>
concept __checks_for = requires(_Key_) {
    { _Key_::template check<_Type_>() };
};

template <
    concepts::settings _S1_, 
    concepts::settings _S2_>
struct is_equivalent: 
    std::bool_constant<is_equivalent<__keywords_t<_S1_, keys>,
        __keywords_t<_S2_, keys>>::value &&  
    is_equivalent<__keywords_t<_S2_, keys>, 
        __keywords_t<_S1_, :keys>>::value>
{};

template <
    concepts::settings _S1_, 
    concepts::settings _S2_>
constexpr inline bool is_equivalent_v = is_equivalent<_S1_, _S2_>::value;

__RAZE_OPTIONS_CONCEPTS_NAMESPACE_END

