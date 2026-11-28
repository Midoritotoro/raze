#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Applies `f` element-wise across one or more equal-size `tuple_like` objects and returns a `raze::meta::tuple` of the results.
 *
 * Evaluates in guaranteed left-to-right order (`0 .. N-1`).
 *
 * @tparam F Callable accepting `(member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed results with decayed/unwrapped element types.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto map(F&& f, T&& t, Ts&& ... ts) {
    return __build_sequenced<size_v<T>>([&] (auto i) -> decltype(auto) {
        return __invoke(
            f, __adl_get<i.value>(std::forward<T>(t)),
            __adl_get<i.value>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Alias for `map(f, t, ts...)`.
 * @tparam F Transformation callable type.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed results.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto transform(F&& f, T&& t, Ts&& ... ts) {
    return meta::map(std::forward<F>(f), std::forward<T>(t), std::forward<Ts>(ts)...);
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Applies `f` element-wise in reverse index order (`N-1 .. 0`), returning a `tuple` whose `i`-th element comes from index `N - 1 - i`.
 * @tparam F Transformation callable type.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed elements in reversed order.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto map_reversed(F&& f, T&& t, Ts&& ... ts) {
    constexpr sizetype n = size_v<T>;

    return __build_sequenced<n>([&] (auto i) -> decltype(auto) {
        constexpr sizetype r = n - 1 - i.value;
        return __invoke(
            f, __adl_get<r>(std::forward<T>(t)),
            __adl_get<r>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Alias for `map_reversed(f, t, ts...)`.
 * @tparam F Transformation callable type.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed elements in reversed order.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto transform_reversed(F&& f, T&& t, Ts&& ... ts) {
    return meta::map_reversed(std::forward<F>(f), std::forward<T>(t), std::forward<Ts>(ts)...);
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Applies `f(index<I>, get<I>(t), get<I>(ts)...)` for each `I` in `0 .. N-1` and collects the results into a `tuple`.
 * @tparam F Callable accepting `(index_t<I>, member_t<I, T>, member_t<I, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Indexed transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed results.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto map_index(F&& f, T&& t, Ts&& ... ts) {
    return __build_sequenced<size_v<T>>([&] (auto i) -> decltype(auto) {
        return __invoke(
            f, i, __adl_get<i.value>(std::forward<T>(t)),
            __adl_get<i.value>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Applies `f(index<R>, get<R>(t), get<R>(ts)...)` for `R = N-1` down to `0` and collects the results into a `tuple`.
 * @tparam F Callable accepting `(index_t<R>, member_t<R, T>, member_t<R, Ts>...)`.
 * @tparam T First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Indexed transformation callable.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of transformed results in reversed order.
 */
template <class F, tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr auto map_index_reversed(F&& f, T&& t, Ts&& ... ts) {
    constexpr sizetype n = size_v<T>;

    return __build_sequenced<n>([&] (auto i) -> decltype(auto) {
        constexpr sizetype r = n - 1 - i.value;
        return __invoke(
            f, index<r>, __adl_get<r>(std::forward<T>(t)),
            __adl_get<r>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Applies binary callable `f` to every consecutive pair `(get<I>(t), get<I + 1>(t))`, returning an `(N - 1)`-element `tuple`.
 *
 * Returns `tuple<>{}` if `size_v<T> < 2`.
 *
 * @tparam F Binary callable type.
 * @tparam T A `tuple_like` type.
 * @param f Callable applied to each adjacent pair.
 * @param t Source tuple-like object.
 * @return `tuple` of pairwise results (empty if `size_v<T> < 2`).
 */
template <class F, tuple_like T>
raze_nodiscard constexpr auto adjacent_map(F&& f, T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n < 2) {
        return tuple<>{};
    }
    else {
        return __build_sequenced<n - 1>([&] (auto i) -> decltype(auto) {
            return __invoke(
                f, __adl_get<i.value>(std::forward<T>(t)),
                __adl_get<i.value + 1>(std::forward<T>(t)));
        });
    }
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Converts every element of `t` to `Target` via `static_cast<Target>`.
 * @tparam Target Destination element type.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Homogeneous `tuple<Target, ..., Target>` of size `size_v<T>`.
 */
template <class Target, tuple_like T>
raze_nodiscard constexpr auto member_cast(T&& t) {
    return __build_sequenced<size_v<T>>([&] (auto i) {
        return static_cast<Target>(__adl_get<i.value>(std::forward<T>(t)));
    });
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Element-wise conditional selection between `on_true` and `on_false` controlled by boolean `mask`.
 * @tparam Mask `tuple_like` of elements contextually convertible to `bool`.
 * @tparam A `tuple_like` of true-branch values.
 * @tparam B `tuple_like` of false-branch values (same size as `Mask` and `A`).
 * @param mask Condition tuple.
 * @param on_true Values chosen where `bool(mask[i])` is `true`.
 * @param on_false Values chosen where `bool(mask[i])` is `false`.
 * @return `tuple` whose `i`-th element is `bool(mask[i]) ? on_true[i] : on_false[i]`.
 */
template <tuple_like Mask, tuple_like A, tuple_like B>
    requires (same_size_tuples<Mask, A, B>)
raze_nodiscard constexpr auto select(Mask&& mask, A&& on_true, B&& on_false) {
    return meta::map([] (auto&& m, auto&& a, auto&& b) {
        return static_cast<bool>(m)
            ? std::forward<decltype(a)>(a)
            : std::forward<decltype(b)>(b);
    }, std::forward<Mask>(mask), std::forward<A>(on_true), std::forward<B>(on_false));
}

/**
 * @ingroup raze_meta_tuple_transform
 * @brief Element-wise custom blend/selection applying `blend_op(mask[i], on_true[i], on_false[i])` (useful for SIMD masks).
 * @tparam Mask `tuple_like` of mask values (e.g., `raze::vx::simd_mask`).
 * @tparam A `tuple_like` of true-branch values.
 * @tparam B `tuple_like` of false-branch values (same size as `Mask` and `A`).
 * @tparam BlendOp Ternary blending callable.
 * @param mask Mask tuple.
 * @param on_true True-branch tuple.
 * @param on_false False-branch tuple.
 * @param blend_op Custom per-element selector/blender.
 * @return `tuple` of blended results.
 */
template <tuple_like Mask, tuple_like A, tuple_like B, class BlendOp>
    requires (same_size_tuples<Mask, A, B>)
raze_nodiscard constexpr auto select(Mask&& mask, A&& on_true, B&& on_false, BlendOp&& blend_op) {
    return meta::map(std::forward<BlendOp>(blend_op),
        std::forward<Mask>(mask), std::forward<A>(on_true), std::forward<B>(on_false));
}

__RAZE_META_NAMESPACE_END
