#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f` on corresponding elements across one or more equal-size `tuple_like` objects in ascending index order (`0 .. N-1`).
 * @tparam F Callable accepting `(member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types with the same size as `T`.
 * @param f Callable invoked for each element index.
 * @param t First tuple-like object.
 * @param ts Additional tuple-like objects.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
constexpr void for_each(F&& f, T&& t, Ts&& ... ts) {
    if constexpr (size_v<T> > 0) {
        auto step = [&] <sizetype I> () raze_always_inline_lambda {
            static_cast<void>(__invoke(
                f, __adl_get<I>(std::forward<T>(t)), __adl_get<I>(std::forward<Ts>(ts))...));
        };

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (step.template operator()<Is>(), ...);
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f` on corresponding elements across one or more equal-size `tuple_like` objects in reverse index order (`N-1 .. 0`).
 * @tparam F Callable accepting `(member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types with the same size as `T`.
 * @param f Callable invoked for each element index in reverse order.
 * @param t First tuple-like object.
 * @param ts Additional tuple-like objects.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
constexpr void for_each_reversed(F&& f, T&& t, Ts&& ... ts) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n > 0) {
        auto step = [&] <sizetype I> () raze_always_inline_lambda {
            static_cast<void>(__invoke(
                f, __adl_get<n - 1 - I>(std::forward<T>(t)),
                __adl_get<n - 1 - I>(std::forward<Ts>(ts))...));
        };

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (step.template operator()<Is>(), ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f(index<I>, get<I>(t), get<I>(ts)...)` for each `I` from `0` to `N-1`.
 * @tparam F Callable accepting `(index_t<I>, member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types with the same size as `T`.
 * @param f Callable invoked with the compile-time `index<I>` tag and corresponding elements.
 * @param t First tuple-like object.
 * @param ts Additional tuple-like objects.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
constexpr void for_each_index(F&& f, T&& t, Ts&& ... ts) {
    if constexpr (size_v<T> > 0) {
        auto step = [&] <sizetype I> () raze_always_inline_lambda {
            static_cast<void>(__invoke(
                f, index<I>, __adl_get<I>(std::forward<T>(t)),
                __adl_get<I>(std::forward<Ts>(ts))...));
        };

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (step.template operator()<Is>(), ...);
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f(index<I>, get<I>(t), get<I>(ts)...)` in reverse index order (`N-1` down to `0`).
 * @tparam F Callable accepting `(index_t<I>, member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types with the same size as `T`.
 * @param f Callable invoked with the compile-time `index<I>` tag and corresponding elements.
 * @param t First tuple-like object.
 * @param ts Additional tuple-like objects.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
constexpr void for_each_index_reversed(F&& f, T&& t, Ts&& ... ts) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n > 0) {
        auto step = [&] <sizetype I> () raze_always_inline_lambda {
            static_cast<void>(__invoke(
                f, index<n - 1 - I>,
                __adl_get<n - 1 - I>(std::forward<T>(t)),
                __adl_get<n - 1 - I>(std::forward<Ts>(ts))...));
        };

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (step.template operator()<Is>(), ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f(get<I>(t), get<I + 1>(t))` for every adjacent element pair from `I = 0` to `N - 2`.
 *
 * Does nothing if `size_v<T> < 2`.
 *
 * @tparam F Binary callable type.
 * @tparam T A `tuple_like` type.
 * @param f Callable invoked on each consecutive pair.
 * @param t Source tuple-like object.
 */
template <class F, tuple_like T>
constexpr void adjacent_for_each(F&& f, T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n >= 2) {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (static_cast<void>(__invoke(
                f, __adl_get<Is>(std::forward<T>(t)), __adl_get<Is + 1>(std::forward<T>(t)))), ...);
        }(std::make_index_sequence<n - 1>{});
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Invokes `f(get<I>(t), get<I + 1>(t))` for every adjacent pair in reverse order (`I = N - 2` down to `0`).
 *
 * Does nothing if `size_v<T> < 2`.
 *
 * @tparam F Binary callable type.
 * @tparam T A `tuple_like` type.
 * @param f Callable invoked on each consecutive pair from right to left.
 * @param t Source tuple-like object.
 */
template <class F, tuple_like T>
constexpr void adjacent_for_each_reversed(F&& f, T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n >= 2) {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (static_cast<void>(__invoke(
                f, __adl_get<n - 2 - Is>(std::forward<T>(t)),
                __adl_get<n - 1 - Is>(std::forward<T>(t)))), ...);
        }(std::make_index_sequence<n - 1>{});
    }
}

template <bool PassIndex, sizetype B, sizetype E, class F, class T>
constexpr decltype(auto) __visit_range(sizetype idx, F&& f, T&& t) {
    constexpr sizetype len = E - B;

    if constexpr (len == 1) {
        if constexpr (PassIndex)
            return __invoke(std::forward<F>(f), index<B>, __adl_get<B>(std::forward<T>(t)));
        else
            return __invoke(std::forward<F>(f), __adl_get<B>(std::forward<T>(t)));
    }
    else {
        constexpr sizetype mid = B + len / 2;

        if (idx < mid)
            return __visit_range<PassIndex, B, mid>(idx, std::forward<F>(f), std::forward<T>(t));
        else
            return __visit_range<PassIndex, mid, E>(idx, std::forward<F>(f), std::forward<T>(t));
    }
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Dispatches runtime index `idx` via a compile-time binary search tree to invoke `f(get<idx>(t))`.
 *
 * @warning Precondition: `idx < size_v<T>`. Out-of-bounds indices invoke `std::unreachable()` (UB).
 *          Use `try_visit_at` when bounds checking is required.
 *
 * @tparam F Visitor callable returning a common type across all element types of `T`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param idx Runtime zero-based element index (`idx < size_v<T>`).
 * @param f Visitor callable.
 * @param t Source tuple-like object.
 * @return The result of invoking `f` on the selected element.
 */
template <class F, non_empty_tuple_like T>
constexpr decltype(auto) visit_at(sizetype idx, F&& f, T&& t) {
    if (idx >= size_v<T>)
        std::unreachable();

    return __visit_range<false, 0, size_v<T>>(idx, std::forward<F>(f), std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Dispatches runtime index `idx` to invoke `f(index<I>, get<I>(t))` where `I == idx`.
 *
 * @warning Precondition: `idx < size_v<T>`. Out-of-bounds indices invoke `std::unreachable()` (UB).
 *
 * @tparam F Visitor callable accepting `(index_t<I>, member_t<I, T>)`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param idx Runtime zero-based element index (`idx < size_v<T>`).
 * @param f Visitor callable receiving both the compile-time `index<I>` tag and element reference.
 * @param t Source tuple-like object.
 * @return The result of invoking `f`.
 */
template <class F, non_empty_tuple_like T>
constexpr decltype(auto) visit_at_index(sizetype idx, F&& f, T&& t) {
    if (idx >= size_v<T>)
        std::unreachable();

    return __visit_range<true, 0, size_v<T>>(idx, std::forward<F>(f), std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_visit
 * @brief Bounds-checked runtime-index visitor that invokes `f(get<idx>(t))` if `idx < size_v<T>`.
 * @tparam F Visitor callable type.
 * @tparam T A `tuple_like` type (empty tuples are permitted and always return `false`).
 * @param idx Runtime zero-based element index.
 * @param f Visitor callable invoked when `idx` is in bounds.
 * @param t Source tuple-like object.
 * @return `true` if `idx < size_v<T>` and `f` was invoked; `false` otherwise.
 */
template <class F, tuple_like T>
constexpr bool try_visit_at(sizetype idx, F&& f, T&& t) {
    if constexpr (size_v<T> == 0) {
        return false;
    }
    else {
        if (idx >= size_v<T>)
            return false;

        __visit_range<false, 0, size_v<T>>(idx, std::forward<F>(f), std::forward<T>(t));
        return true;
    }
}

__RAZE_META_NAMESPACE_END
