#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Creates a homogeneous `N`-element `tuple` with every element initialized to `v`.
 * @tparam N Number of elements to generate.
 * @tparam T Element value type.
 * @param v Value replicated into every slot.
 * @return `repeat_tuple_t<std::unwrap_ref_decay_t<const T&>, N>` filled with `v`.
 */
template <sizetype N, class T>
raze_nodiscard constexpr auto fill(const T& v) {
    return __build_sequenced<N>([&] (auto) -> const T& {
        return v;
    });
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Tag-dispatched overload of `fill<N>(v)` accepting an `index_t<N>` tag (e.g., `fill(4_c, 7)`).
 * @tparam N Number of elements to generate.
 * @tparam T Element value type.
 * @return `N`-element `tuple` filled with `v`.
 */
template <sizetype N, class T>
raze_nodiscard constexpr auto fill(index_t<N>, const T& v) {
    return meta::fill<N>(v);
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Broadcasts `v` into an `N`-element homogeneous `tuple` (alias for `fill<N>(v)`).
 * @tparam N Number of elements.
 * @tparam T Element value type.
 * @param v Value to broadcast.
 * @return `N`-element `tuple` filled with `v`.
 */
template <sizetype N, class T>
raze_nodiscard constexpr auto splat(const T& v) {
    return meta::fill<N>(v);
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Tag-dispatched overload of `splat<N>(v)` accepting an `index_t<N>` tag (e.g., `splat(4_c, 7)`).
 * @tparam N Number of elements.
 * @tparam T Element value type.
 * @return `N`-element `tuple` filled with `v`.
 */
template <sizetype N, class T>
raze_nodiscard constexpr auto splat(index_t<N>, const T& v) {
    return meta::fill<N>(v);
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Builds an `N`-element `tuple` by invoking `f(index<I>)` for `I = 0 .. N-1` in guaranteed left-to-right order.
 * @tparam N Number of elements to generate.
 * @tparam F Generator callable accepting `index_t<I>`.
 * @param f Generator callable.
 * @return `tuple` containing `(f(index<0>), ..., f(index<N-1>))`.
 */
template <sizetype N, class F>
raze_nodiscard constexpr auto generate(F&& f) {
    return __build_sequenced<N>([&] (auto i) -> decltype(auto) {
        return __invoke(f, i);
    });
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Tag-dispatched overload of `generate<N>(f)` accepting an `index_t<N>` tag.
 * @tparam N Number of elements to generate.
 * @tparam F Generator callable accepting `index_t<I>`.
 * @return `tuple` containing `(f(index<0>), ..., f(index<N-1>))`.
 */
template <sizetype N, class F>
raze_nodiscard constexpr auto generate(index_t<N>, F&& f) {
    return meta::generate<N>(std::forward<F>(f));
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Builds an `N`-element `tuple` by invoking `f(index<N - 1 - I>)` for `I = 0 .. N-1`.
 * @tparam N Number of elements to generate.
 * @tparam F Generator callable accepting `index_t<R>`.
 * @param f Generator callable.
 * @return `tuple` containing `(f(index<N-1>), ..., f(index<0>))`.
 */
template <sizetype N, class F>
raze_nodiscard constexpr auto generate_reversed(F&& f) {
    return __build_sequenced<N>([&] (auto i) -> decltype(auto) {
        return __invoke(f, index<N - 1 - i.value>);
    });
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Tag-dispatched overload of `generate_reversed<N>(f)` accepting an `index_t<N>` tag.
 * @tparam N Number of elements to generate.
 * @tparam F Generator callable accepting `index_t<R>`.
 * @return `tuple` containing `(f(index<N-1>), ..., f(index<0>))`.
 */
template <sizetype N, class F>
raze_nodiscard constexpr auto generate_reversed(index_t<N>, F&& f) {
    return meta::generate_reversed<N>(std::forward<F>(f));
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Builds an `N`-element homogeneous `tuple<T, ..., T>` containing sequentially increasing values `start, start + 1, ..., start + N - 1`.
 * @tparam N Number of elements.
 * @tparam T Arithmetic element type (defaults to `sizetype`).
 * @param start Starting value (defaults to `T{0}`).
 * @return Homogeneous `tuple` of ascending values.
 */
template <sizetype N, class T = sizetype>
raze_nodiscard constexpr auto iota(T start = T{ 0 }) {
    return __build_sequenced<N>([start] (auto i) {
        return static_cast<T>(start + static_cast<T>(i.value));
    });
}

/**
 * @ingroup raze_meta_tuple_generate
 * @brief Tag-dispatched overload of `iota<N, T>(start)` accepting an `index_t<N>` tag (e.g., `iota(4_c, 10)`).
 * @tparam N Number of elements.
 * @tparam T Arithmetic element type (defaults to `sizetype`).
 * @return Homogeneous `tuple` of ascending values starting at `start`.
 */
template <sizetype N, class T = sizetype>
raze_nodiscard constexpr auto iota(index_t<N>, T start = T{ 0 }) {
    return meta::iota<N, T>(start);
}

__RAZE_META_NAMESPACE_END
