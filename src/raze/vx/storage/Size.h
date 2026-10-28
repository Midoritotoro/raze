#pragma once 

#include <src/raze/vx/storage/Node.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class>
struct simd_tuple_size_impl;

template <>
struct simd_tuple_size_impl<simd_tuple_nil>:
    std::integral_constant<sizetype, 0>
{};

template <class H, class T>
struct simd_tuple_size_impl<simd_tuple_node<H, T>>:
    std::integral_constant<sizetype, 1 + simd_tuple_size_impl<T>::value>
{};

template <class T>
struct simd_tuple_size:
    simd_tuple_size_impl<std::remove_cvref_t<T>>
{};

template <class T, sizetype N>
struct simd_tuple_repeat_;

template <class T>
struct simd_tuple_repeat_<T, 0> { using type = simd_tuple_nil; };

template <class T, sizetype N>
struct simd_tuple_repeat_ {
    using type = simd_tuple_node<T, typename simd_tuple_repeat_<T, N - 1>::type>;
};

template <class T, sizetype N>
using simd_tuple_repeat = typename simd_tuple_repeat_<T, N>::type;

template <sizetype I, class Tuple>
struct simd_tuple_element_;

template <class H, class T>
struct simd_tuple_element_<0, simd_tuple_node<H, T>> { using type = H; };

template <sizetype I, class H, class T>
struct simd_tuple_element_<I, simd_tuple_node<H, T>>
    : simd_tuple_element_<I - 1, T> {};

template <sizetype I, class Tuple>
using simd_tuple_element_t =
    typename simd_tuple_element_<I, std::remove_cvref_t<Tuple>>::type;

template <class Tuple>
inline constexpr sizetype simd_tuple_size_v = simd_tuple_size<Tuple>::value;

template <class A, class B>
struct simd_tuple_cat_impl;

template <class B>
struct simd_tuple_cat_impl<simd_tuple_nil, B> {
    using type = std::remove_cvref_t<B>;
};

template <class H, class T, class B>
struct simd_tuple_cat_impl<simd_tuple_node<H, T>, B> {
    using type = simd_tuple_node<H, typename simd_tuple_cat_impl<T, B>::type>;
};

template <class A, class B>
using simd_tuple_cat_t = typename simd_tuple_cat_impl<
    std::remove_cvref_t<A>, std::remove_cvref_t<B>>::type;

template <class A, class B>
raze_always_inline simd_tuple_cat_t<A, B> simd_tuple_cat(A&& a, B&& b) noexcept {
    if constexpr (std::is_same_v<std::remove_cvref_t<A>, simd_tuple_nil>) return std::forward<B>(b);
    else return simd_tuple_cat_t<A, B>{ std::forward<A>(a)._head, simd_tuple_cat(a._tail, std::forward<B>(b)) };
}

template <class T>
raze_always_inline auto as_simd_tuple(T&& t) noexcept {
    if constexpr (is_simd_tuple<std::remove_cvref_t<T>>::value) return std::forward<T>(t);
    else return simd_tuple_node<std::remove_cvref_t<T>>{std::forward<T>(t)};
}

raze_always_inline simd_tuple_nil simd_tuple_make() noexcept { return {}; }

template <class H, class... Ts>
raze_always_inline auto simd_tuple_make(H&& h, Ts&&... ts) noexcept {
    return simd_tuple_node<std::remove_cvref_t<H>, std::remove_cvref_t<
        decltype(simd_tuple_make(std::forward<Ts>(ts)...))>> { std::forward<H>(h), simd_tuple_make(std::forward<Ts>(ts)...) };
}


__RAZE_VX_NAMESPACE_END