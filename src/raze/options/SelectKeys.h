#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Settings.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Key, concepts::keyword ... Keywords>
struct filter {
    using type = keys<Keywors...>;

    template <class T> 
    constexpr raze_always_inline auto operator+(const keys<T>&) const noexcept {
        using kw_t = typename T::keyword_type;

        if constexpr (!std::same_as<Key, typename kw_t::tag_type>) return filter<Key, Keywors..., kw_t>{};
        else return *this;
    }
};

template <class Key, class Select>
struct select_keys;

template <class Key, concepts::option ... Options>
struct select_keys<Key, settings<Options...>> :
    decltype((filter<typename Key::tag_type>{} + ... + keys<Options>{}))
{};

__RAZE_OPTIONS_NAMESPACE_END
