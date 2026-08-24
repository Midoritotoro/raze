#pragma once 


#include <src/raze/traits/TypeTraits.h>


__RAZE_VX_NAMESPACE_BEGIN

struct simd_tuple_nil 
{};

template <class H, class T = simd_tuple_nil>
struct simd_tuple_node {
    raze_no_unique_address H _head;
    raze_no_unique_address T _tail;

    raze_always_inline simd_tuple_node() noexcept = default;
    raze_always_inline simd_tuple_node(const simd_tuple_node&) noexcept = default;
    raze_always_inline simd_tuple_node(simd_tuple_node&&) noexcept = default;

    raze_always_inline ~simd_tuple_node() = default;

    raze_always_inline simd_tuple_node& operator=(const simd_tuple_node&) noexcept = default;
    raze_always_inline simd_tuple_node& operator=(simd_tuple_node&&) noexcept = default;

    template <class NewH, class NewT>
    raze_always_inline _Simd_tuple_node(NewH&& head, NewT&& tail) noexcept:
        _head(std::forward<NewH>(head)), 
        _tail(std::forward<NewT>(tail))
    {}
};

template <class T> 
struct is_simd_tuple: 
    std::false_type 
{};

template <class H, class T> 
struct is_simd_tuple<simd_tuple_node<H, T>>: 
    std::true_type 
{};

template <> 
struct is_simd_tuple<simd_tuple_nil>:
    std::true_type 
{};

template <class T>
concept wrapped_type = requires(const T& x) { x.data(); };

template <class T>
raze_always_inline auto ustorage(const T& x) noexcept {
    if constexpr (wrapped_type<T>) return x.data();
    else return x;
}

template <class T>
raze_always_inline auto& ustorage(T& x) noexcept {
    if constexpr (wrapped_type<T>) return x.data();
    else return x;
}

__RAZE_VX_NAMESPACE_END
