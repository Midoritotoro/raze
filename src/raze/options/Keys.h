#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <type_traits>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class ... Keys> 
struct keys {};

template <class Key, class Keys> 
struct contains;

template <class ... Keys, class Key>
struct contains<Key, keys<Keys...>>: 
    std::bool_constant<(std::same_as<Key, Keys> || ...)>
{};

template <class Key, class Keys, bool> 
struct append_if_impl;

template <class ... Keys, class Key> 
struct append_if_impl<Key, keys<Keys...>, true> {
    using type = keys<Keys...>;
};

template <class ... Keys, class Key> 
struct append_if_impl<Key, keys<Keys...>, false> {
    using type = keys<Keys..., Key>;
};

template <class Key, class Keys> 
struct append_if;

template <class Key, class Keys> 
struct append_if: 
    append_if_impl<Key, Keys, contains<Key, Keys>::value>
{};

template <class Key1, class Key2> 
struct uniques;

template <class Keys1, class Key2, class ... Keys2>
struct uniques<Keys1, keys<Key2, Keys2...>>:
    uniques<typename append_if<Key2, Keys1>::type, keys<Keys2...>>
{};

template <class Keys1> 
struct uniques<Keys1, keys<>> {
    using type = Keys1; 
};

template <class Key1, class Key2> 
struct contain_all;

template <class Key1, class Key2>
struct contain_all;

template <class Keys1, class ... Keys2>
struct contain_all<Keys1, keys<Keys2...>>: 
    std::bool_constant<(contains<Keys2, Keys1>::value && ...)>
{};

template <class Keys1> 
struct contain_all<Keys1, keys<>>: 
    std::false_type 
{};

template <class Keys2>
struct contain_all<keys<>, Keys2>:
    std::false_type 
{};

template <> 
struct contain_all<keys<>, keys<>>:
    std::true_type  
{};

template <class Key1, class Key2>
struct is_equivalent: 
    std::bool_constant<contain_all<Key2, Key1>::value && contain_all<Key2, Key1>::value> 
{};

__RAZE_OPTIONS_NAMESPACE_END
