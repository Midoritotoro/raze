#pragma once

#include <src/raze/RazeNamespace.h>

#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/compatibility/CxxVersionDetection.h>

#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class T, class ... Ts>
constexpr inline bool is_any_of_v = (std::same_as<T, Ts> || ...);

template <class T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

namespace detail {
    template <class T>
    void swap(T&, T&) = delete;

    template <class T>
    concept has_adl_swap = requires(T& a, T& b) {
        swap(a, b);
    };
}

template <class T>
struct is_trivially_swappable;

template <class T>
constexpr inline bool is_trivially_swappable_v = is_trivially_swappable<T>::value;

template <class T>
struct is_trivially_swappable : std::bool_constant<
    std::is_object_v<T> && std::swappable<T> &&
    !std::is_const_v<T> && !std::is_volatile_v<T> &&
    !detail::has_adl_swap<std::remove_all_extents_t<T>> &&
    std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>> &&
    std::is_trivially_move_assignable_v<std::remove_all_extents_t<T>> &&
    std::is_trivially_destructible_v<std::remove_all_extents_t<T>>>
{};

template <class T1, class T2>
struct is_trivially_swappable<std::pair<T1, T2>> : std::bool_constant<
    is_trivially_swappable_v<T1> && is_trivially_swappable_v<T2>>
{};

template <class T, std::size_t N>
struct is_trivially_swappable<std::array<T, N>> : std::bool_constant<
    is_trivially_swappable_v<T>>
{};

template <class ... Ts>
struct is_trivially_swappable<std::tuple<Ts...>> : std::bool_constant<
    (is_trivially_swappable_v<Ts> && ...)>
{};

template <class T>
concept trivially_swappable = is_trivially_swappable_v<T>;

__RAZE_TRAITS_NAMESPACE_END