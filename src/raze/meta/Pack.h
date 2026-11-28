#pragma once

#include <raze/Types.h>
#include <src/raze/compatibility/Compatibility.h>
#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>


__RAZE_META_NAMESPACE_BEGIN

#if __has_builtin(__type_pack_element)
template <sizetype I, class ... Ts>
using __nth_type_t = __type_pack_element<I, Ts...>;
#else
template <sizetype I, class T>
struct __type_entry {};

template <class Seq, class ... Ts>
struct __type_map;

template <sizetype ... Is, class ... Ts>
struct __type_map<std::index_sequence<Is...>, Ts...> :
    __type_entry<Is, Ts>...
{};

template <sizetype I, class T>
consteval auto __lookup_type(__type_entry<I, T>) -> std::type_identity<T>;

template <sizetype I, class ... Ts>
using __nth_type_t = typename decltype(__lookup_type<I>(
    __type_map<std::index_sequence_for<Ts...>, Ts...>{}))::type;
#endif

template <class Target, class ... Ts>
consteval sizetype __count_type_in_pack() noexcept {
    return (static_cast<sizetype>(std::is_same_v<Target, Ts>) + ... + sizetype{ 0 });
}

template <class Target, class ... Ts>
consteval sizetype __index_of_unique_type() noexcept {
    if constexpr (sizeof...(Ts) == 0) {
        return 0;
    }
    else {
        sizetype found = sizeof...(Ts);
        sizetype count = 0;
        sizetype i = 0;

        (((std::is_same_v<Target, Ts> ? (found = i, ++count, 0) : 0), ++i), ...);
        return count == 1 ? found : sizeof...(Ts);
    }
}

template <class ... Ts>
inline constexpr bool __no_refs_v = (!std::is_reference_v<Ts> && ...);

template <class ... Ts>
inline constexpr bool __has_empty_v = (std::is_empty_v<Ts> || ...);

template <class ... Ts>
inline constexpr bool __all_same_v = false;

template <class T0, class ... Ts>
inline constexpr bool __all_same_v<T0, Ts...> = (std::is_same_v<T0, Ts> && ...);

template <class T, sizetype>
using __repeat_elem_t = T;

template <sizetype ... Idxs>
consteval bool __indices_unique() noexcept {
    constexpr sizetype n = sizeof...(Idxs);

    if constexpr (n <= 1) {
        return true;
    }
    else {
        constexpr sizetype arr[n] = { Idxs... };

        for (sizetype i = 0; i < n; ++i) {
            for (sizetype j = i + 1; j < n; ++j) {
                if (arr[i] == arr[j])
                    return false;
            }
        }

        return true;
    }
}

template <sizetype Pos, sizetype ... Idxs>
consteval sizetype __nth_index() noexcept {
    constexpr sizetype arr[sizeof...(Idxs)] = { Idxs... };
    return arr[Pos];
}

template <sizetype Pos, sizetype ... Idxs>
consteval bool __is_last_occurrence() noexcept {
    constexpr sizetype n = sizeof...(Idxs);
    constexpr sizetype arr[n] = { Idxs... };
    constexpr sizetype target = arr[Pos];

    for (sizetype i = Pos + 1; i < n; ++i) {
        if (arr[i] == target)
            return false;
    }

    return true;
}

template <class MemberType, class Self, class M>
raze_always_inline constexpr decltype(auto) __forward_member(M& m) noexcept {
    if constexpr (std::is_lvalue_reference_v<MemberType>) {
        return static_cast<MemberType>(m);
    }
    else if constexpr (std::is_rvalue_reference_v<MemberType>) {
        if constexpr (std::is_lvalue_reference_v<Self>)
            return static_cast<std::remove_reference_t<MemberType>&>(m);
        else
            return static_cast<MemberType>(m);
    }
    else {
        using clean_self = std::remove_reference_t<Self>;
        using qual_t = std::conditional_t<std::is_const_v<clean_self>, const MemberType, MemberType>;

        if constexpr (std::is_lvalue_reference_v<Self>)
            return static_cast<qual_t&>(m);
        else
            return static_cast<qual_t&&>(m);
    }
}

template <class F, class ... Args>
raze_always_inline constexpr decltype(auto) __invoke(F&& f, Args&& ... args)
    noexcept(std::is_nothrow_invocable_v<F, Args...>)
    requires (std::invocable<F, Args...>)
{
    if constexpr (std::is_member_pointer_v<std::remove_cvref_t<F>>)
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    else
        return std::forward<F>(f)(std::forward<Args>(args)...);
}

struct __synth_three_way_fn {
    template <class T, class U>
        requires (std::three_way_comparable_with<T, U>)
    raze_static_operator raze_always_inline constexpr auto operator()(const T& a, const U& b) raze_const_operator
        noexcept(noexcept(a <=> b))
    {
        return a <=> b;
    }

    template <class T, class U>
        requires (!std::three_way_comparable_with<T, U>) &&
                 requires (const T& a, const U& b) {
                     { a < b } -> std::convertible_to<bool>;
                     { b < a } -> std::convertible_to<bool>;
                 }
    raze_static_operator raze_always_inline constexpr std::weak_ordering operator()(const T& a, const U& b) raze_const_operator
        noexcept(noexcept(a < b) && noexcept(b < a))
    {
        if (a < b)
            return std::weak_ordering::less;

        if (b < a)
            return std::weak_ordering::greater;

        return std::weak_ordering::equivalent;
    }
};

inline constexpr __synth_three_way_fn __synth_three_way{};

template <sizetype I, class T>
raze_always_inline constexpr decltype(auto) __adl_get(T&& t) noexcept {
    using std::get;
    return get<I>(std::forward<T>(t));
}

__RAZE_META_NAMESPACE_END

