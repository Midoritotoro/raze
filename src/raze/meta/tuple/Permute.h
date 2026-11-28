#pragma once

#include <src/raze/meta/tuple/Structural.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Reorders, selects, or duplicates elements of `t` according to compile-time indices `Idxs...`.
 *
 * Move-safe even when indices repeat and `t` is an rvalue: earlier occurrences copy from `t` as
 * an lvalue and only the final occurrence of each index forwards (`std::forward<T>(t)`). When all
 * indices are unique, moves/forwards every slot directly in a single pass.
 *
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple<element_t<Idxs, T>...>`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard constexpr auto reorder(T&& t) {
    if constexpr (sizeof...(Idxs) == 0) {
        return tuple<>{};
    }
    else if constexpr (__indices_unique<Idxs...>()) {
        return meta::make_tuple_exact<element_t<Idxs, T>...>(__adl_get<Idxs>(std::forward<T>(t))...);
    }
    else {
        return [&] <sizetype ... Pos> (std::index_sequence<Pos...>) raze_always_inline_lambda {
            return __build_typed_sequenced<element_t<Idxs, T>...>([&] (auto p) -> decltype(auto) {
                constexpr sizetype idx = __nth_index<p.value, Idxs...>();

                if constexpr (__is_last_occurrence<p.value, Idxs...>())
                    return __adl_get<idx>(std::forward<T>(t));
                else
                    return __adl_get<idx>(t);
            });
        }(std::make_index_sequence<sizeof...(Idxs)>{});
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Overload of `reorder<Idxs...>(t)` accepting a `std::index_sequence<Idxs...>`.
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple<element_t<Idxs, T>...>`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard constexpr auto reorder(T&& t, std::index_sequence<Idxs...>) {
    return meta::reorder<Idxs...>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `reorder<Idxs...>(t)` accepting `index_t<Idxs>...` tags (e.g., `reorder(t, 3_c, 0_c, 3_c)`).
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple<element_t<Idxs, T>...>`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard constexpr auto reorder(T&& t, index_t<Idxs>...) {
    return meta::reorder<Idxs...>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Creates a zero-copy reference view reordering `t` according to `Idxs...`.
 *
 * When indices repeat, binds lvalue references (`get<Idxs>(t)`) to avoid multiple rvalue-reference
 * aliases to the same subobject.
 *
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references ordered by `Idxs...`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard raze_always_inline constexpr auto reorder_ref(T&& t) noexcept {
    if constexpr (sizeof...(Idxs) == 0)
        return tuple<>{};
    else if constexpr (__indices_unique<Idxs...>())
        return meta::forward_as_tuple(__adl_get<Idxs>(std::forward<T>(t))...);
    else
        return meta::forward_as_tuple(__adl_get<Idxs>(t)...);
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Overload of `reorder_ref<Idxs...>(t)` accepting a `std::index_sequence<Idxs...>`.
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references ordered by `Idxs...`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard raze_always_inline constexpr auto reorder_ref(T&& t, std::index_sequence<Idxs...>) noexcept {
    return meta::reorder_ref<Idxs...>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `reorder_ref<Idxs...>(t)` accepting `index_t<Idxs>...` tags.
 * @tparam Idxs Output source indices (each `< size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references ordered by `Idxs...`.
 */
template <sizetype ... Idxs, tuple_like T>
    requires ((Idxs < size_v<T>) && ...)
raze_nodiscard raze_always_inline constexpr auto reorder_ref(T&& t, index_t<Idxs>...) noexcept {
    return meta::reorder_ref<Idxs...>(std::forward<T>(t));
}

template <class Pattern, sizetype I, sizetype N>
consteval sizetype __eval_pattern_index() noexcept {
    if constexpr (std::invocable<Pattern, index_t<I>, index_t<N>>)
        return static_cast<sizetype>(Pattern{}(index<I>, index<N>));
    else
        return static_cast<sizetype>(Pattern{}(index<I>));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Compile-time pattern permutation producing an `OutN`-element `tuple`.
 *
 * Evaluates stateless `Pattern` at compile time for each output slot `I = 0 .. OutN - 1` via
 * `Pattern{}(index<I>, index<size_v<T>>)` (or `Pattern{}(index<I>)` for unary patterns) and
 * dispatches directly to `reorder`.
 *
 * ### Example
 * ```cpp
 * constexpr auto gathered = raze::meta::permute<6>(t, [](auto i, auto n) { return i % n; });
 * ```
 *
 * @tparam OutN Output tuple length.
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @return Permuted `tuple` of length `OutN`.
 */
template <sizetype OutN, tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard constexpr auto permute(T&& t, Pattern&&) {
    using pattern_t = std::remove_cvref_t<Pattern>;
    constexpr sizetype in_n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder<__eval_pattern_index<pattern_t, Is, in_n>()...>(std::forward<T>(t));
    }(std::make_index_sequence<OutN>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Same-size compile-time pattern permutation (`OutN == size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @param t Source tuple-like object.
 * @param pattern Compile-time permutation pattern callable.
 * @return Permuted `tuple` of length `size_v<T>`.
 */
template <tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard constexpr auto permute(T&& t, Pattern&& pattern) {
    return meta::permute<size_v<T>>(std::forward<T>(t), std::forward<Pattern>(pattern));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched compile-time pattern permutation specifying output size via `index_t<OutN>` (e.g., `permute(t, 2_c, pattern)`).
 * @tparam OutN Output tuple length.
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @return Permuted `tuple` of length `OutN`.
 */
template <sizetype OutN, tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard constexpr auto permute(T&& t, index_t<OutN>, Pattern&& pattern) {
    return meta::permute<OutN>(std::forward<T>(t), std::forward<Pattern>(pattern));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zero-copy reference-view version of `permute<OutN>(t, pattern)`.
 * @tparam OutN Output view length.
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @return Permuted reference `tuple` of length `OutN`.
 */
template <sizetype OutN, tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard raze_always_inline constexpr auto permute_ref(T&& t, Pattern&&) noexcept {
    using pattern_t = std::remove_cvref_t<Pattern>;
    constexpr sizetype in_n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder_ref<__eval_pattern_index<pattern_t, Is, in_n>()...>(std::forward<T>(t));
    }(std::make_index_sequence<OutN>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Same-size zero-copy reference-view version of `permute(t, pattern)`.
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @param t Source tuple-like object.
 * @param pattern Compile-time permutation pattern callable.
 * @return Permuted reference `tuple` of length `size_v<T>`.
 */
template <tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard raze_always_inline constexpr auto permute_ref(T&& t, Pattern&& pattern) noexcept {
    return meta::permute_ref<size_v<T>>(std::forward<T>(t), std::forward<Pattern>(pattern));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched zero-copy reference-view version of `permute(t, index<OutN>, pattern)`.
 * @tparam OutN Output view length.
 * @tparam T A `tuple_like` type.
 * @tparam Pattern Stateless, default-constructible compile-time index mapping callable.
 * @return Permuted reference `tuple` of length `OutN`.
 */
template <sizetype OutN, tuple_like T, class Pattern>
    requires (std::is_empty_v<std::remove_cvref_t<Pattern>> &&
              std::is_default_constructible_v<std::remove_cvref_t<Pattern>>)
raze_nodiscard raze_always_inline constexpr auto permute_ref(T&& t, index_t<OutN>, Pattern&& pattern) noexcept {
    return meta::permute_ref<OutN>(std::forward<T>(t), std::forward<Pattern>(pattern));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Returns a new `tuple` with the elements of `t` in reverse order.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Reversed `tuple`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto reverse(T&& t) {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder<(n - 1 - Is)...>(std::forward<T>(t));
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Creates a zero-copy forwarding-reference view with the elements of `t` in reverse order.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Reversed reference `tuple`.
 */
template <tuple_like T>
raze_nodiscard raze_always_inline constexpr auto reverse_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder_ref<(n - 1 - Is)...>(std::forward<T>(t));
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Cyclically rotates `t` left by `R` positions (modulo `size_v<T>`).
 * @tparam R Number of positions to rotate left.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Rotated `tuple` (or `tuple<>{}` if empty).
 */
template <sizetype R, tuple_like T>
raze_nodiscard constexpr auto rotate_left(T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return tuple<>{};
    }
    else {
        constexpr sizetype s = R % n;

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return meta::reorder<((Is + s) % n)...>(std::forward<T>(t));
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `rotate_left<R>(t)` accepting an `index_t<R>` tag.
 * @tparam R Number of positions to rotate left.
 * @tparam T A `tuple_like` type.
 * @return Rotated `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard constexpr auto rotate_left(T&& t, index_t<R>) {
    return meta::rotate_left<R>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Creates a zero-copy forwarding-reference view cyclically rotated left by `R` positions.
 * @tparam R Number of positions to rotate left.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Rotated reference `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto rotate_left_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return tuple<>{};
    }
    else {
        constexpr sizetype s = R % n;

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return meta::reorder_ref<((Is + s) % n)...>(std::forward<T>(t));
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `rotate_left_ref<R>(t)` accepting an `index_t<R>` tag.
 * @tparam R Number of positions to rotate left.
 * @tparam T A `tuple_like` type.
 * @return Rotated reference `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto rotate_left_ref(T&& t, index_t<R>) noexcept {
    return meta::rotate_left_ref<R>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Cyclically rotates `t` right by `R` positions (modulo `size_v<T>`).
 * @tparam R Number of positions to rotate right.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Rotated `tuple` (or `tuple<>{}` if empty).
 */
template <sizetype R, tuple_like T>
raze_nodiscard constexpr auto rotate_right(T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0)
        return tuple<>{};
    else
        return meta::rotate_left<(n - (R % n)) % n>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `rotate_right<R>(t)` accepting an `index_t<R>` tag.
 * @tparam R Number of positions to rotate right.
 * @tparam T A `tuple_like` type.
 * @return Rotated `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard constexpr auto rotate_right(T&& t, index_t<R>) {
    return meta::rotate_right<R>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Creates a zero-copy forwarding-reference view cyclically rotated right by `R` positions.
 * @tparam R Number of positions to rotate right.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Rotated reference `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto rotate_right_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0)
        return tuple<>{};
    else
        return meta::rotate_left_ref<(n - (R % n)) % n>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `rotate_right_ref<R>(t)` accepting an `index_t<R>` tag.
 * @tparam R Number of positions to rotate right.
 * @tparam T A `tuple_like` type.
 * @return Rotated reference `tuple`.
 */
template <sizetype R, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto rotate_right_ref(T&& t, index_t<R>) noexcept {
    return meta::rotate_right_ref<R>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Swaps every adjacent pair of `GroupSize`-element blocks (butterfly swap).
 *
 * For `GroupSize = 1`: `(a, b, c, d)` -> `(b, a, d, c)`.
 * For `GroupSize = 2`: `(a, b, c, d)` -> `(c, d, a, b)`.
 *
 * @tparam GroupSize Block size in elements (`GroupSize > 0` and `size_v<T> % (2 * GroupSize) == 0`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` with adjacent groups swapped.
 */
template <sizetype GroupSize = 1, tuple_like T>
    requires (GroupSize > 0 && (size_v<T> % (2 * GroupSize) == 0))
raze_nodiscard constexpr auto swap_adjacent(T&& t) {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder<(((Is / GroupSize) ^ sizetype{ 1 }) * GroupSize + (Is % GroupSize))...>(std::forward<T>(t));
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `swap_adjacent<GroupSize>(t)` accepting an `index_t<GroupSize>` tag.
 * @tparam GroupSize Block size in elements.
 * @tparam T A `tuple_like` type.
 * @return `tuple` with adjacent groups swapped.
 */
template <sizetype GroupSize, tuple_like T>
    requires (GroupSize > 0 && (size_v<T> % (2 * GroupSize) == 0))
raze_nodiscard constexpr auto swap_adjacent(T&& t, index_t<GroupSize>) {
    return meta::swap_adjacent<GroupSize>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zero-copy reference-view version of `swap_adjacent<GroupSize>(t)`.
 * @tparam GroupSize Block size in elements (`GroupSize > 0` and `size_v<T> % (2 * GroupSize) == 0`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Reference `tuple` with adjacent groups swapped.
 */
template <sizetype GroupSize = 1, tuple_like T>
    requires (GroupSize > 0 && (size_v<T> % (2 * GroupSize) == 0))
raze_nodiscard raze_always_inline constexpr auto swap_adjacent_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::reorder_ref<(((Is / GroupSize) ^ sizetype{ 1 }) * GroupSize + (Is % GroupSize))...>(std::forward<T>(t));
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Tag-dispatched overload of `swap_adjacent_ref<GroupSize>(t)` accepting an `index_t<GroupSize>` tag.
 * @tparam GroupSize Block size in elements.
 * @tparam T A `tuple_like` type.
 * @return Reference `tuple` with adjacent groups swapped.
 */
template <sizetype GroupSize, tuple_like T>
    requires (GroupSize > 0 && (size_v<T> % (2 * GroupSize) == 0))
raze_nodiscard raze_always_inline constexpr auto swap_adjacent_ref(T&& t, index_t<GroupSize>) noexcept {
    return meta::swap_adjacent_ref<GroupSize>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zips one or more equal-size `tuple_like` objects into a `tuple` of row tuples `(make_tuple(t0[i], ts[i]...)...)`.
 * @tparam T0 First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T0`.
 * @param t0 First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of zipped row tuples.
 */
template <tuple_like T0, tuple_like ... Ts>
    requires (same_size_tuples<T0, Ts...>)
raze_nodiscard constexpr auto zip(T0&& t0, Ts&& ... ts) {
    return __build_sequenced<size_v<T0>>([&] (auto i) {
        return meta::make_tuple(
            __adl_get<i.value>(std::forward<T0>(t0)),
            __adl_get<i.value>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zero-copy reference-view version of `zip(t0, ts...)`.
 * @tparam T0 First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T0`.
 * @param t0 First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of reference-tuple rows.
 */
template <tuple_like T0, tuple_like ... Ts>
    requires (same_size_tuples<T0, Ts...>)
raze_nodiscard raze_always_inline constexpr auto zip_ref(T0&& t0, Ts&& ... ts) noexcept {
    return __build_sequenced<size_v<T0>>([&] (auto i) {
        return meta::forward_as_tuple(
            __adl_get<i.value>(std::forward<T0>(t0)),
            __adl_get<i.value>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zips `tuple_like` objects of potentially differing lengths, truncating to `min(size_v<T0>, size_v<Ts>...)`.
 * @tparam T0 First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types.
 * @param t0 First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return `tuple` of zipped row tuples with length equal to the shortest input.
 */
template <tuple_like T0, tuple_like ... Ts>
raze_nodiscard constexpr auto zip_min(T0&& t0, Ts&& ... ts) {
    constexpr sizetype min_n = [] {
        sizetype m = size_v<T0>;
        ((m = (size_v<Ts> < m) ? size_v<Ts> : m), ...);
        return m;
    }();

    return __build_sequenced<min_n>([&] (auto i) {
        return meta::make_tuple(
            __adl_get<i.value>(std::forward<T0>(t0)),
            __adl_get<i.value>(std::forward<Ts>(ts))...);
    });
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Transposes a tuple of equal-length inner tuples (row-major to column-major).
 * @tparam T A `tuple_like` of `tuple_like` rows.
 * @param t Source matrix-like tuple.
 * @return Transposed `tuple` of column tuples (or `tuple<>{}` if `t` is empty).
 */
template <tuple_like T>
raze_nodiscard constexpr auto transpose(T&& t) {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        constexpr sizetype inner_n = size_v<element_t<0, T>>;

        return __build_sequenced<inner_n>([&] (auto col) {
            return __build_sequenced<size_v<T>>([&] (auto row) -> decltype(auto) {
                return __adl_get<col.value>(__adl_get<row.value>(std::forward<T>(t)));
            });
        });
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zero-copy reference-view transposition of a tuple of equal-length inner tuples.
 * @tparam T A `tuple_like` of `tuple_like` rows.
 * @param t Source matrix-like tuple.
 * @return Transposed `tuple` of column reference tuples.
 */
template <tuple_like T>
raze_nodiscard raze_always_inline constexpr auto transpose_ref(T&& t) noexcept {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        constexpr sizetype inner_n = size_v<element_t<0, T>>;

        return __build_sequenced<inner_n>([&] (auto col) {
            return [&] <sizetype ... Rows> (std::index_sequence<Rows...>) raze_always_inline_lambda {
                return meta::forward_as_tuple(__adl_get<col.value>(__adl_get<Rows>(std::forward<T>(t)))...);
            }(std::make_index_sequence<size_v<T>>{});
        });
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Inverse of `zip`: unzips a tuple of row tuples into a tuple of column tuples (alias for `transpose`).
 * @tparam T A `tuple_like` of `tuple_like` rows.
 * @param t Source zipped tuple.
 * @return Tuple of unzipped column tuples.
 */
template <tuple_like T>
raze_nodiscard constexpr auto unzip(T&& t) {
    return meta::transpose(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Zero-copy reference-view inverse of `zip` (alias for `transpose_ref`).
 * @tparam T A `tuple_like` of `tuple_like` rows.
 * @param t Source zipped tuple.
 * @return Tuple of unzipped column reference tuples.
 */
template <tuple_like T>
raze_nodiscard raze_always_inline constexpr auto unzip_ref(T&& t) noexcept {
    return meta::transpose_ref(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Flattens one level of nested `tuple_like` elements inside `t`, preserving non-tuple elements as-is.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Single-level flattened `tuple`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto flatten(T&& t) {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        auto wrap = [] <class V> (V&& v) -> decltype(auto) {
            if constexpr (tuple_like<V>)
                return std::forward<V>(v);
            else
                return meta::make_tuple(std::forward<V>(v));
        };

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return meta::cat(wrap(__adl_get<Is>(std::forward<T>(t)))...);
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Recursively flattens all nested `tuple_like` layers inside `t` into a single 1D `tuple`.
 * @tparam T A `tuple_like` type.
 * @param t Source arbitrarily nested tuple-like object.
 * @return Fully flattened `tuple` of non-tuple leaf elements.
 */
template <tuple_like T>
raze_nodiscard constexpr auto flatten_all(T&& t) {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        auto wrap = [] <class V> (V&& v) {
            if constexpr (tuple_like<V>)
                return meta::flatten_all(std::forward<V>(v));
            else
                return meta::make_tuple(std::forward<V>(v));
        };

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return meta::cat(wrap(__adl_get<Is>(std::forward<T>(t)))...);
        }(std::make_index_sequence<size_v<T>>{});
    }
}

template <class ... Ts>
consteval auto __build_cartesian_table() noexcept {
    constexpr sizetype k = sizeof...(Ts);
    constexpr sizetype total = (size_v<Ts> * ... * sizetype{ 1 });

    struct coord {
        sizetype c[k];
    };

    struct table {
        coord data[total];
    } out{};

    constexpr sizetype dims[k] = { size_v<Ts>... };

    for (sizetype n = 0; n < total; ++n) {
        sizetype rem = n;

        for (sizetype d = k; d-- > 0;) {
            out.data[n].c[d] = rem % dims[d];
            rem /= dims[d];
        }
    }

    return out;
}

/**
 * @ingroup raze_meta_tuple_permute
 * @brief Computes the lexicographical Cartesian product of zero or more `tuple_like` objects.
 *
 * Returns `tuple<>{}` if no tuples are supplied or if any input tuple is empty.
 *
 * @tparam Ts Zero or more `tuple_like` types.
 * @param ts Source tuple-like factors.
 * @return `tuple` of `sizeof...(Ts)`-tuples covering every combination in lexicographical order.
 */
template <tuple_like ... Ts>
raze_nodiscard constexpr auto cartesian_product(Ts&& ... ts) {
    constexpr sizetype k = sizeof...(Ts);
    constexpr sizetype total = (k == 0) ? 0 : (size_v<Ts> * ... * sizetype{ 1 });

    if constexpr (total == 0) {
        return tuple<>{};
    }
    else {
        constexpr auto coords = __build_cartesian_table<Ts...>();
        auto refs = meta::forward_as_tuple(std::forward<Ts>(ts)...);

        return __build_sequenced<total>([&] (auto n) {
            return [&] <sizetype ... Ds> (std::index_sequence<Ds...>) raze_always_inline_lambda {
                return meta::make_tuple(
                    __adl_get<coords.data[n.value].c[Ds]>(refs.template get<Ds>())...);
            }(std::make_index_sequence<k>{});
        });
    }
}

__RAZE_META_NAMESPACE_END
