#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/options/SameAs.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class ... _Keys_> 
struct keys 
{};

template <
    class _Key_,
    class _Keys_> 
struct contains;

template <
    class ...   _Keys_,
    class       _Key_>
struct contains<_Key_, keys<_Keys_...>>: 
    std::bool_constant<(concepts::same_as<_Key_, _Keys_> || ...)>
{};

template <
    class _Key_,
    class _Keys_, 
    bool> 
struct __append_if;

template <
    class ...   _Keys_,
    class       _Key_> 
struct __append_if<_Key_, keys<_Keys_...>, true> {
    using type = keys<_Keys_...>;
};

template <
    class ...   _Keys_,
    class       _Key_> 
struct __append_if<_Key_, keys<_Keys_...>, false> {
    using type = keys<_Keys_..., _Key_>;
};

template <
    class _Key_,
    class _Keys_> 
struct append_if;

template <
    class _Key_,
    class _Keys_> 
struct append_if: 
    __append_if<_Key_, _Keys_, contains<_Key_, _Keys_>::value>
{};

template <
    class _Key1_, 
    class _Key2_> 
struct uniques;

template <
    class       _Keys1_, 
    class       _Key2_, 
    class ...   _Keys2_>
struct uniques<_Keys1_, keys<_Key2_,_Keys2_...>>: 
    uniques<typename append_if<_Key2_,_Keys1_>::type, keys<_Keys2_...>>
{};

template <class _Keys1_> 
struct uniques<_Keys1_, keys<>> {
    using type = _Keys1_; 
};

template <
    class _Key1_, 
    class _Key2_> 
struct contain_all;

template <
    class _Key1_,
    class _Key2_>
struct contain_all;

template <
    class       _Keys1_,
    class ...   _Keys2_>
struct contain_all<_Keys1_, keys<_Keys2_...>>: 
    std::bool_constant<(contains<_Keys2_,_Keys1_>::value && ...)>
{};

template <class _Keys1_> 
struct contain_all<_Keys1_, keys<>>: 
    std::false_type 
{};

template <class _Keys2_>
struct contain_all<keys<>, _Keys2_>:
    std::false_type 
{};

template <> 
struct contain_all<keys<>, keys<>>:
    std::true_type  
{};

template <
    class _Key1_,
    class _Key2_>
struct is_equivalent: 
    std::bool_constant<contain_all<_Key2_, _Key1_>::value && contain_all<_Key2_, _Key1_>::value> 
{};

__RAZE_OPTIONS_NAMESPACE_END
