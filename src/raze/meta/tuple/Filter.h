#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

template <template <class> class Pred, tuple_like T>
consteval auto __build_partition_table() noexcept {
    constexpr sizetype n = size_v<T>;

    struct table {
        sizetype cut = 0;
        sizetype idx[n + 1]{};
    } tbl{};

    [&] <sizetype ... Is> (std::index_sequence<Is...>) {
        sizetype p = 0;
        ((Pred<element_t<Is, T>>::value ? (tbl.idx[p++] = Is, 0) : 0), ...);
        tbl.cut = p;
        ((!Pred<element_t<Is, T>>::value ? (tbl.idx[p++] = Is, 0) : 0), ...);
    }(std::make_index_sequence<n>{});

    return tbl;
}

template <tuple_like T>
consteval auto __build_adjacent_unique_table() noexcept {
    constexpr sizetype n = size_v<T>;

    struct table {
        sizetype count = 0;
        sizetype idx[n + 1]{};
    } tbl{};

    if constexpr (n > 0) {
        tbl.idx[tbl.count++] = 0;

        [&] <sizetype ... Is> (std::index_sequence<Is...>) {
            ((!std::is_same_v<element_t<Is, T>, element_t<Is + 1, T>>
                ? (tbl.idx[tbl.count++] = Is + 1, 0) : 0), ...);
        }(std::make_index_sequence<n - 1>{});
    }

    return tbl;
}

template <tuple_like T>
consteval auto __build_dedup_table() noexcept {
    constexpr sizetype n = size_v<T>;

    struct table {
        sizetype count = 0;
        sizetype idx[n + 1]{};
    } tbl{};

    auto add_if_new = [&] <sizetype I> () {
        bool dup = false;

        [&] <sizetype ... Prev> (std::index_sequence<Prev...>) {
            dup = (std::is_same_v<element_t<I, T>, element_t<Prev, T>> || ...);
        }(std::make_index_sequence<I>{});

        if (!dup)
            tbl.idx[tbl.count++] = I;
    };

    [&] <sizetype ... Is> (std::index_sequence<Is...>) {
        (add_if_new.template operator()<Is>(), ...);
    }(std::make_index_sequence<n>{});

    return tbl;
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Extracts all elements of `t` whose declared element type satisfies `Pred<E>::value` in a single flat pass.
 * @tparam Pred Unary type-trait template exposing `static constexpr bool value`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing only the matching elements in their original relative order.
 */
template <template <class> class Pred, tuple_like T>
raze_nodiscard constexpr auto filter(T&& t) {
    constexpr auto tbl = __build_partition_table<Pred, T>();

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<tbl.idx[Is], T>...>([&] (auto i) -> decltype(auto) {
            return __adl_get<tbl.idx[i.value]>(std::forward<T>(t));
        });
    }(std::make_index_sequence<tbl.cut>{});
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Creates a zero-copy forwarding-reference view of elements in `t` satisfying `Pred<E>::value`.
 * @tparam Pred Unary type-trait template exposing `static constexpr bool value`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to the matching elements.
 */
template <template <class> class Pred, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto filter_ref(T&& t) noexcept {
    constexpr auto tbl = __build_partition_table<Pred, T>();

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<tbl.idx[Is]>(std::forward<T>(t))...);
    }(std::make_index_sequence<tbl.cut>{});
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Extracts all elements of `t` whose declared element type does not satisfy `Pred<E>::value`.
 * @tparam Pred Unary type-trait template exposing `static constexpr bool value`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing only the non-matching elements in their original relative order.
 */
template <template <class> class Pred, tuple_like T>
raze_nodiscard constexpr auto filter_not(T&& t) {
    constexpr auto tbl = __build_partition_table<Pred, T>();
    constexpr sizetype rest = size_v<T> - tbl.cut;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<tbl.idx[tbl.cut + Is], T>...>(
            [&] (auto i) -> decltype(auto) {
                return __adl_get<tbl.idx[tbl.cut + i.value]>(std::forward<T>(t));
            });
    }(std::make_index_sequence<rest>{});
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Creates a zero-copy forwarding-reference view of elements in `t` not satisfying `Pred<E>::value`.
 * @tparam Pred Unary type-trait template exposing `static constexpr bool value`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to the non-matching elements.
 */
template <template <class> class Pred, tuple_like T>
raze_nodiscard raze_always_inline constexpr auto filter_not_ref(T&& t) noexcept {
    constexpr auto tbl = __build_partition_table<Pred, T>();
    constexpr sizetype rest = size_v<T> - tbl.cut;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<tbl.idx[tbl.cut + Is]>(std::forward<T>(t))...);
    }(std::make_index_sequence<rest>{});
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Partitions `t` into a 2-tuple `(matching, non_matching)` according to compile-time type predicate `Pred<E>::value`.
 *
 * Move-safe: each element of `t` is forwarded exactly once into either the first or second partition.
 *
 * @tparam Pred Unary type-trait template exposing `static constexpr bool value`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return 2-element `tuple` of `(matching_tuple, non_matching_tuple)`.
 */
template <template <class> class Pred, tuple_like T>
raze_nodiscard constexpr auto partition(T&& t) {
    constexpr auto tbl = __build_partition_table<Pred, T>();

    auto take_part = [&] <sizetype Offset, sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<tbl.idx[Offset + Is], T>...>(
            [&] (auto i) -> decltype(auto) {
                return __adl_get<tbl.idx[Offset + i.value]>(std::forward<T>(t));
            });
    };

    return meta::make_tuple(
        take_part.template operator()<0>(std::make_index_sequence<tbl.cut>{}),
        take_part.template operator()<tbl.cut>(std::make_index_sequence<size_v<T> - tbl.cut>{}));
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Removes consecutive elements with identical declared types (analogous to `std::unique` on types), keeping the first element of each run.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` with adjacent duplicate types collapsed.
 */
template <tuple_like T>
raze_nodiscard constexpr auto unique_types(T&& t) {
    constexpr auto tbl = __build_adjacent_unique_table<T>();

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<tbl.idx[Is], T>...>([&] (auto i) -> decltype(auto) {
            return __adl_get<tbl.idx[i.value]>(std::forward<T>(t));
        });
    }(std::make_index_sequence<tbl.count>{});
}

/**
 * @ingroup raze_meta_tuple_filter
 * @brief Removes all globally duplicate element types from `t`, retaining only the first occurrence of each distinct type.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing at most one element per distinct type in `T`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto dedup_types(T&& t) {
    constexpr auto tbl = __build_dedup_table<T>();

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<tbl.idx[Is], T>...>([&] (auto i) -> decltype(auto) {
            return __adl_get<tbl.idx[i.value]>(std::forward<T>(t));
        });
    }(std::make_index_sequence<tbl.count>{});
}

__RAZE_META_NAMESPACE_END
