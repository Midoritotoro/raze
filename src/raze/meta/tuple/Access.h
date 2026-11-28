#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_access
 * @brief Creates a zero-copy forwarding-reference tuple view over all elements of `t`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references returned by `get<I>(std::forward<T>(t))`.
 */
template <tuple_like T>
raze_nodiscard raze_always_inline constexpr auto to_ref(T&& t) noexcept {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<Is>(std::forward<T>(t))...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Creates a zero-copy forwarding-reference tuple view over the elements of `t` in reverse order.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to `t` ordered from `size_v<T> - 1` down to `0`.
 */
template <tuple_like T>
raze_nodiscard raze_always_inline constexpr auto reversed_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<n - 1 - Is>(std::forward<T>(t))...);
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Materializes any `tuple_like` object (`std::tuple`, `std::pair`, `std::array`, etc.) into a `raze::meta::tuple`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return A `raze::meta::tuple` containing decayed/unwrapped copies or moves of the elements of `t`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto to_tuple(T&& t) {
    return __build_sequenced<size_v<T>>([&] (auto i) -> decltype(auto) {
        return __adl_get<i.value>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Normalizes any value to a `raze::meta::tuple`.
 *
 * - If `T` is already a `raze::meta::tuple`, forwards `x` unchanged without copying.
 * - Else if `T` satisfies `tuple_like`, converts it via `to_tuple`.
 * - Otherwise, wraps `x` in a 1-element tuple via `make_tuple(std::forward<T>(x))`.
 *
 * @tparam T Any type (tuple, tuple-like, or scalar).
 * @param x Input object.
 * @return Forwarded `raze::meta::tuple` or newly constructed `raze::meta::tuple`.
 */
template <class T>
raze_nodiscard constexpr decltype(auto) as_tuple(T&& x) {
    if constexpr (is_tuple_v<T>)
        return std::forward<T>(x);
    else if constexpr (tuple_like<T>)
        return meta::to_tuple(std::forward<T>(x));
    else
        return meta::make_tuple(std::forward<T>(x));
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Direct-initializes `Target` using the unpacked elements of `t` (`Target(get<Is>(t)...)`).
 * @tparam Target Type to construct.
 * @tparam T A `tuple_like` type.
 * @param t Tuple-like source of constructor arguments.
 * @return Constructed `Target` object.
 */
template <class Target, tuple_like T>
raze_nodiscard constexpr Target make_from_tuple(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return Target(__adl_get<Is>(std::forward<T>(t))...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Aggregate/brace-initializes `Target` using the unpacked elements of `t` (`Target{get<Is>(t)...}`).
 * @tparam Target Aggregate or brace-constructible type to initialize.
 * @tparam T A `tuple_like` type.
 * @param t Tuple-like source of initializer elements.
 * @return Constructed `Target` object.
 */
template <class Target, tuple_like T>
raze_nodiscard constexpr Target from_tuple(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return Target{ __adl_get<Is>(std::forward<T>(t))... };
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Returns a reference to the first element (`index 0`) of a non-empty tuple-like object.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Forwarded reference to `get<0>(t)`.
 */
template <non_empty_tuple_like T>
raze_nodiscard raze_always_inline constexpr decltype(auto) front(T&& t) noexcept {
    return __adl_get<0>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Returns a reference to the last element (`size_v<T> - 1`) of a non-empty tuple-like object.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Forwarded reference to `get<size_v<T> - 1>(t)`.
 */
template <non_empty_tuple_like T>
raze_nodiscard raze_always_inline constexpr decltype(auto) back(T&& t) noexcept {
    return __adl_get<size_v<T> - 1>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_access
 * @brief Invokes callable `f` with the unpacked elements of `t` as arguments.
 *
 * Supports functors, lambdas, function pointers, and member pointers, propagating exact
 * `noexcept` specifications and return value categories.
 *
 * @tparam F Callable type.
 * @tparam T A `tuple_like` type.
 * @param f Callable to invoke.
 * @param t Tuple-like object supplying the arguments.
 * @return The result of invoking `f` with `get<0>(t), ..., get<N-1>(t)`.
 */
template <class F, tuple_like T>
constexpr decltype(auto) apply(F&& f, T&& t)
    noexcept([] <sizetype ... Is> (std::index_sequence<Is...>) {
        return std::is_nothrow_invocable_v<F, member_t<Is, T>...>;
    }(std::make_index_sequence<size_v<T>>{}))
{
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
        return __invoke(std::forward<F>(f), __adl_get<Is>(std::forward<T>(t))...);
    }(std::make_index_sequence<size_v<T>>{});
}

__RAZE_META_NAMESPACE_END
