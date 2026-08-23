#pragma once 

#include <src/raze/options/Keys.h>
#include <concepts>


__RAZE_OPTIONS_CONCEPTS_NAMESPACE_BEGIN

template <class Key>
concept keyword = requires(Key key) {
    typename Key::tag_type;
    { Key::template accept<int>() } -> std::same_as<bool>;
};

template <class Option>
concept option = requires(const Option& opt) {
    { opt(typename std::remove_cvref_t<Option>::keyword_type{}) }
        -> std::same_as<typename std::remove_cvref_t<Option>::stored_value_type>;
};

template <class Settings> 
concept settings = requires(const Settings&) {
    typename Settings::settings_type;
};

template <class Option, auto Keyword>
concept exactly = std::same_as<typename Option::keyword_type, 
    std::remove_cvref_t<decltype(Keyword)>>;

template <class Key, class T>
concept checks_for = requires(Key) {
    { Key::template check<T>() };
};

__RAZE_OPTIONS_CONCEPTS_NAMESPACE_END

