#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

template <class ... Ts>
consteval auto __build_cat_table() noexcept {
    constexpr sizetype total = (size_v<Ts> + ... + sizetype{ 0 });

    struct entry {
        sizetype ti;
        sizetype ei;
    };

    struct table {
        entry data[total];
    } out{};

    constexpr sizetype sizes[] = { size_v<Ts>... };
    sizetype pos = 0;

    for (sizetype ti = 0; ti < sizeof...(Ts); ++ti) {
        for (sizetype ei = 0; ei < sizes[ti]; ++ei)
            out.data[pos++] = { ti, ei };
    }

    return out;
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Concatenates zero or more `tuple_like` objects into a single flat `raze::meta::tuple` in one pass.
 *
 * Uses a compile-time lookup table to move/copy every element directly into its final slot without
 * intermediate recursive tuple construction. Returns `tuple<>{}` when all inputs are empty or no
 * arguments are provided.
 *
 * @tparam Ts Zero or more `tuple_like` types.
 * @param ts Source tuple-like objects to concatenate.
 * @return Concatenated `raze::meta::tuple`.
 */
template <tuple_like ... Ts>
raze_nodiscard constexpr auto cat(Ts&& ... ts) {
    constexpr sizetype total = (size_v<Ts> + ... + sizetype{ 0 });

    if constexpr (total == 0) {
        return tuple<>{};
    }
    else {
        constexpr auto tbl = __build_cat_table<Ts...>();
        auto refs = meta::forward_as_tuple(std::forward<Ts>(ts)...);

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return __build_typed_sequenced<
                element_t<tbl.data[Is].ei, __nth_type_t<tbl.data[Is].ti, Ts...>>...
            >([&] (auto k) -> decltype(auto) {
                constexpr auto e = tbl.data[k.value];
                return __adl_get<e.ei>(std::move(refs).template get<e.ti>());
            });
        }(std::make_index_sequence<total>{});
    }
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference tuple view concatenating all elements of `ts...`.
 * @tparam Ts Zero or more `tuple_like` types.
 * @param ts Source tuple-like objects.
 * @return `tuple` of references spanning every element across `ts...` in order.
 */
template <tuple_like ... Ts>
raze_nodiscard raze_always_inline constexpr auto cat_ref(Ts&& ... ts) noexcept {
    constexpr sizetype total = (size_v<Ts> + ... + sizetype{ 0 });

    if constexpr (total == 0) {
        return tuple<>{};
    }
    else {
        constexpr auto tbl = __build_cat_table<Ts...>();
        auto refs = meta::forward_as_tuple(std::forward<Ts>(ts)...);

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return meta::forward_as_tuple(
                __adl_get<tbl.data[Is].ei>(std::move(refs).template get<tbl.data[Is].ti>())...);
        }(std::make_index_sequence<total>{});
    }
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Extracts the half-open sub-tuple `[Begin, End)` from `t`.
 * @tparam Begin Inclusive start index (`Begin <= End`).
 * @tparam End Exclusive end index (`End <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing elements `Begin .. End - 1`.
 */
template <sizetype Begin, sizetype End, tuple_like T>
    requires (Begin <= End && End <= size_v<T>)
raze_nodiscard constexpr auto slice(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<Begin + Is, T>...>([&] (auto i) -> decltype(auto) {
            return __adl_get<Begin + i.value>(std::forward<T>(t));
        });
    }(std::make_index_sequence<End - Begin>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view of the half-open range `[Begin, End)` in `t`.
 * @tparam Begin Inclusive start index (`Begin <= End`).
 * @tparam End Exclusive end index (`End <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to elements `Begin .. End - 1`.
 */
template <sizetype Begin, sizetype End, tuple_like T>
    requires (Begin <= End && End <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto slice_ref(T&& t) noexcept {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<Begin + Is>(std::forward<T>(t))...);
    }(std::make_index_sequence<End - Begin>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched sub-tuple extraction over `[Begin, End)` (e.g., `extract(t, 1_c, 4_c)`).
 * @tparam Begin Inclusive start index (`Begin <= End`).
 * @tparam End Exclusive end index (`End <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` containing elements `Begin .. End - 1`.
 */
template <sizetype Begin, sizetype End, tuple_like T>
    requires (Begin <= End && End <= size_v<T>)
raze_nodiscard constexpr auto extract(T&& t, index_t<Begin>, index_t<End>) {
    return meta::slice<Begin, End>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched suffix extraction over `[Begin, size_v<T>)` (e.g., `extract(t, 2_c)`).
 * @tparam Begin Inclusive start index (`Begin <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` containing elements `Begin .. size_v<T> - 1`.
 */
template <sizetype Begin, tuple_like T>
    requires (Begin <= size_v<T>)
raze_nodiscard constexpr auto extract(T&& t, index_t<Begin>) {
    return meta::slice<Begin, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched zero-copy reference view over `[Begin, End)` (e.g., `extract_ref(t, 1_c, 4_c)`).
 * @tparam Begin Inclusive start index (`Begin <= End`).
 * @tparam End Exclusive end index (`End <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references to elements `Begin .. End - 1`.
 */
template <sizetype Begin, sizetype End, tuple_like T>
    requires (Begin <= End && End <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto extract_ref(T&& t, index_t<Begin>, index_t<End>) noexcept {
    return meta::slice_ref<Begin, End>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched zero-copy reference view over `[Begin, size_v<T>)` (e.g., `extract_ref(t, 2_c)`).
 * @tparam Begin Inclusive start index (`Begin <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references to elements `Begin .. size_v<T> - 1`.
 */
template <sizetype Begin, tuple_like T>
    requires (Begin <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto extract_ref(T&& t, index_t<Begin>) noexcept {
    return meta::slice_ref<Begin, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Returns a `tuple` containing the first `N` elements of `t`.
 * @tparam N Number of leading elements to take (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of elements `[0, N)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard constexpr auto take(T&& t) {
    return meta::slice<0, N>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `take<N>(t)` accepting an `index_t<N>` tag.
 * @tparam N Number of leading elements to take (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of elements `[0, N)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard constexpr auto take(T&& t, index_t<N>) {
    return meta::slice<0, N>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view of the first `N` elements of `t`.
 * @tparam N Number of leading elements (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to elements `[0, N)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto take_ref(T&& t) noexcept {
    return meta::slice_ref<0, N>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `take_ref<N>(t)` accepting an `index_t<N>` tag.
 * @tparam N Number of leading elements (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references to elements `[0, N)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto take_ref(T&& t, index_t<N>) noexcept {
    return meta::slice_ref<0, N>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Returns a `tuple` with the first `N` elements of `t` removed (`[N, size_v<T>)`).
 * @tparam N Number of leading elements to drop (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of elements `[N, size_v<T>)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard constexpr auto drop(T&& t) {
    return meta::slice<N, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `drop<N>(t)` accepting an `index_t<N>` tag.
 * @tparam N Number of leading elements to drop (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of elements `[N, size_v<T>)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard constexpr auto drop(T&& t, index_t<N>) {
    return meta::slice<N, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view skipping the first `N` elements of `t`.
 * @tparam N Number of leading elements to skip (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to elements `[N, size_v<T>)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto drop_ref(T&& t) noexcept {
    return meta::slice_ref<N, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `drop_ref<N>(t)` accepting an `index_t<N>` tag.
 * @tparam N Number of leading elements to skip (`N <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references to elements `[N, size_v<T>)`.
 */
template <sizetype N, tuple_like T>
    requires (N <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto drop_ref(T&& t, index_t<N>) noexcept {
    return meta::slice_ref<N, size_v<T>>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Splits `t` at index `At` into a 2-tuple `(slice<0, At>(t), slice<At, N>(t))`.
 * @tparam At Split position (`At <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return 2-element `tuple` containing the prefix and suffix sub-tuples.
 */
template <sizetype At, tuple_like T>
    requires (At <= size_v<T>)
raze_nodiscard constexpr auto split(T&& t) {
    return meta::make_tuple(
        meta::slice<0, At>(std::forward<T>(t)),
        meta::slice<At, size_v<T>>(std::forward<T>(t)));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `split<At>(t)` accepting an `index_t<At>` tag.
 * @tparam At Split position (`At <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return 2-element `tuple` containing the prefix and suffix sub-tuples.
 */
template <sizetype At, tuple_like T>
    requires (At <= size_v<T>)
raze_nodiscard constexpr auto split(T&& t, index_t<At>) {
    return meta::split<At>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Splits `t` at index `At` into a 2-tuple of zero-copy reference views `(slice_ref<0, At>(t), slice_ref<At, N>(t))`.
 * @tparam At Split position (`At <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return 2-element `tuple` of reference-tuple views.
 */
template <sizetype At, tuple_like T>
    requires (At <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto split_ref(T&& t) noexcept {
    return meta::make_tuple(
        meta::slice_ref<0, At>(std::forward<T>(t)),
        meta::slice_ref<At, size_v<T>>(std::forward<T>(t)));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `split_ref<At>(t)` accepting an `index_t<At>` tag.
 * @tparam At Split position (`At <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return 2-element `tuple` of reference-tuple views.
 */
template <sizetype At, tuple_like T>
    requires (At <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto split_ref(T&& t, index_t<At>) noexcept {
    return meta::split_ref<At>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Splits an even-sized `tuple_like` object into two equal halves `(lo, hi)`.
 * @tparam T A `tuple_like` type with `size_v<T> % 2 == 0`.
 * @param t Source tuple-like object.
 * @return 2-element `tuple` of the lower and upper halves.
 */
template <tuple_like T>
    requires (size_v<T> % 2 == 0)
raze_nodiscard constexpr auto split_half(T&& t) {
    return meta::split<size_v<T> / 2>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Splits an even-sized `tuple_like` object into two equal-size zero-copy reference views `(lo_ref, hi_ref)`.
 * @tparam T A `tuple_like` type with `size_v<T> % 2 == 0`.
 * @param t Source tuple-like object.
 * @return 2-element `tuple` of reference views for the lower and upper halves.
 */
template <tuple_like T>
    requires (size_v<T> % 2 == 0)
raze_nodiscard raze_always_inline constexpr auto split_half_ref(T&& t) noexcept {
    return meta::split_ref<size_v<T> / 2>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Partitions `t` into consecutive non-overlapping chunks of up to `K` elements (the last chunk may be shorter).
 * @tparam K Chunk size (`K > 0`).
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of chunk tuples.
 */
template <sizetype K, non_empty_tuple_like T>
    requires (K > 0)
raze_nodiscard constexpr auto chunks(T&& t) {
    constexpr sizetype n = size_v<T>;
    constexpr sizetype num_chunks = (n + K - 1) / K;

    return __build_sequenced<num_chunks>([&] (auto c) {
        constexpr sizetype b = c.value * K;
        constexpr sizetype e = (b + K < n) ? (b + K) : n;
        return meta::slice<b, e>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `chunks<K>(t)` accepting an `index_t<K>` tag.
 * @tparam K Chunk size (`K > 0`).
 * @tparam T A `non_empty_tuple_like` type.
 * @return `tuple` of chunk tuples.
 */
template <sizetype K, non_empty_tuple_like T>
    requires (K > 0)
raze_nodiscard constexpr auto chunks(T&& t, index_t<K>) {
    return meta::chunks<K>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Partitions `t` into consecutive zero-copy reference-tuple chunks of up to `K` elements.
 * @tparam K Chunk size (`K > 0`).
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of reference-tuple chunks.
 */
template <sizetype K, non_empty_tuple_like T>
    requires (K > 0)
raze_nodiscard raze_always_inline constexpr auto chunks_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;
    constexpr sizetype num_chunks = (n + K - 1) / K;

    return __build_sequenced<num_chunks>([&] (auto c) {
        constexpr sizetype b = c.value * K;
        constexpr sizetype e = (b + K < n) ? (b + K) : n;
        return meta::slice_ref<b, e>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `chunks_ref<K>(t)` accepting an `index_t<K>` tag.
 * @tparam K Chunk size (`K > 0`).
 * @tparam T A `non_empty_tuple_like` type.
 * @return `tuple` of reference-tuple chunks.
 */
template <sizetype K, non_empty_tuple_like T>
    requires (K > 0)
raze_nodiscard raze_always_inline constexpr auto chunks_ref(T&& t, index_t<K>) noexcept {
    return meta::chunks_ref<K>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Produces all overlapping contiguous sliding windows of width `W` over `t`.
 *
 * Copies elements safely even when `t` is an rvalue because elements are shared across adjacent windows.
 *
 * @tparam W Window width (`0 < W <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of `size_v<T> - W + 1` window tuples.
 */
template <sizetype W, tuple_like T>
    requires (W > 0 && W <= size_v<T>)
raze_nodiscard constexpr auto windows(T&& t) {
    constexpr sizetype num_windows = size_v<T> - W + 1;

    return __build_sequenced<num_windows>([&] (auto w) {
        return meta::slice<w.value, w.value + W>(t);
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `windows<W>(t)` accepting an `index_t<W>` tag.
 * @tparam W Window width (`0 < W <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of overlapping window tuples.
 */
template <sizetype W, tuple_like T>
    requires (W > 0 && W <= size_v<T>)
raze_nodiscard constexpr auto windows(T&& t, index_t<W>) {
    return meta::windows<W>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Produces overlapping zero-copy reference-tuple sliding windows of width `W` over `t`.
 * @tparam W Window width (`0 < W <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of `size_v<T> - W + 1` reference-tuple windows.
 */
template <sizetype W, tuple_like T>
    requires (W > 0 && W <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto windows_ref(T&& t) noexcept {
    constexpr sizetype num_windows = size_v<T> - W + 1;

    return __build_sequenced<num_windows>([&] (auto w) {
        return meta::slice_ref<w.value, w.value + W>(t);
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `windows_ref<W>(t)` accepting an `index_t<W>` tag.
 * @tparam W Window width (`0 < W <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of overlapping reference-tuple windows.
 */
template <sizetype W, tuple_like T>
    requires (W > 0 && W <= size_v<T>)
raze_nodiscard raze_always_inline constexpr auto windows_ref(T&& t, index_t<W>) noexcept {
    return meta::windows_ref<W>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Selects every `S`-th element of `t` starting from `Offset`: indices `Offset, Offset + S, Offset + 2*S, ...`.
 *
 * Returns `tuple<>{}` if `Offset >= size_v<T>`.
 *
 * @tparam S Stride step (`S > 0`).
 * @tparam Offset Initial element offset (defaults to `0`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of strided elements.
 */
template <sizetype S, sizetype Offset = 0, tuple_like T>
    requires (S > 0)
raze_nodiscard constexpr auto stride(T&& t) {
    constexpr sizetype n = size_v<T>;
    constexpr sizetype count = (Offset < n) ? ((n - Offset + S - 1) / S) : 0;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<Offset + Is * S, T>...>([&] (auto i) -> decltype(auto) {
            return __adl_get<Offset + i.value * S>(std::forward<T>(t));
        });
    }(std::make_index_sequence<count>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view of every `S`-th element of `t` starting from `Offset`.
 * @tparam S Stride step (`S > 0`).
 * @tparam Offset Initial element offset (defaults to `0`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references to strided elements.
 */
template <sizetype S, sizetype Offset = 0, tuple_like T>
    requires (S > 0)
raze_nodiscard raze_always_inline constexpr auto stride_ref(T&& t) noexcept {
    constexpr sizetype n = size_v<T>;
    constexpr sizetype count = (Offset < n) ? ((n - Offset + S - 1) / S) : 0;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<Offset + Is * S>(std::forward<T>(t))...);
    }(std::make_index_sequence<count>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Interleaves one or more equal-size `tuple_like` objects in round-robin order (`t0[0], t1[0], ..., t0[1], t1[1], ...`).
 * @tparam T0 First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T0`.
 * @param t0 First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return Flat interleaved `tuple` of size `(1 + sizeof...(Ts)) * size_v<T0>`.
 */
template <tuple_like T0, tuple_like ... Ts>
    requires (same_size_tuples<T0, Ts...>)
raze_nodiscard constexpr auto interleave(T0&& t0, Ts&& ... ts) {
    constexpr sizetype k = 1 + sizeof...(Ts);
    constexpr sizetype m = size_v<T0>;
    constexpr sizetype total = k * m;
    auto refs = meta::forward_as_tuple(std::forward<T0>(t0), std::forward<Ts>(ts)...);

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<
            element_t<Is / k, __nth_type_t<Is % k, T0, Ts...>>...
        >([&] (auto i) -> decltype(auto) {
            return __adl_get<i.value / k>(std::move(refs).template get<i.value % k>());
        });
    }(std::make_index_sequence<total>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view interleaving `t0` and `ts...` in round-robin order.
 * @tparam T0 First `tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T0`.
 * @param t0 First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return Flat `tuple` of interleaved element references.
 */
template <tuple_like T0, tuple_like ... Ts>
    requires (same_size_tuples<T0, Ts...>)
raze_nodiscard raze_always_inline constexpr auto interleave_ref(T0&& t0, Ts&& ... ts) noexcept {
    constexpr sizetype k = 1 + sizeof...(Ts);
    constexpr sizetype m = size_v<T0>;
    constexpr sizetype total = k * m;
    auto refs = meta::forward_as_tuple(std::forward<T0>(t0), std::forward<Ts>(ts)...);

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(
            __adl_get<Is / k>(std::move(refs).template get<Is % k>())...);
    }(std::make_index_sequence<total>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Inverse of `interleave`: separates `t` into a `K`-tuple of strided sub-tuples `(stride<K, 0>(t), ..., stride<K, K-1>(t))`.
 * @tparam K Number of interleaved channels (`K > 0`).
 * @tparam T A `tuple_like` type.
 * @param t Source interleaved tuple-like object.
 * @return `K`-element `tuple` of deinterleaved channel tuples.
 */
template <sizetype K, tuple_like T>
    requires (K > 0)
raze_nodiscard constexpr auto deinterleave(T&& t) {
    return __build_sequenced<K>([&] (auto g) {
        return meta::stride<K, g.value>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `deinterleave<K>(t)` accepting an `index_t<K>` tag.
 * @tparam K Number of interleaved channels (`K > 0`).
 * @tparam T A `tuple_like` type.
 * @return `K`-element `tuple` of deinterleaved channel tuples.
 */
template <sizetype K, tuple_like T>
    requires (K > 0)
raze_nodiscard constexpr auto deinterleave(T&& t, index_t<K>) {
    return meta::deinterleave<K>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Zero-copy reference-view inverse of `interleave`: returns a `K`-tuple of `stride_ref<K, g>(t)` views.
 * @tparam K Number of interleaved channels (`K > 0`).
 * @tparam T A `tuple_like` type.
 * @param t Source interleaved tuple-like object.
 * @return `K`-element `tuple` of deinterleaved reference-tuple views.
 */
template <sizetype K, tuple_like T>
    requires (K > 0)
raze_nodiscard raze_always_inline constexpr auto deinterleave_ref(T&& t) noexcept {
    return __build_sequenced<K>([&] (auto g) {
        return meta::stride_ref<K, g.value>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `deinterleave_ref<K>(t)` accepting an `index_t<K>` tag.
 * @tparam K Number of interleaved channels (`K > 0`).
 * @tparam T A `tuple_like` type.
 * @return `K`-element `tuple` of deinterleaved reference-tuple views.
 */
template <sizetype K, tuple_like T>
    requires (K > 0)
raze_nodiscard raze_always_inline constexpr auto deinterleave_ref(T&& t, index_t<K>) noexcept {
    return meta::deinterleave_ref<K>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Prepends `v` to the front of `t` in a single flat pass.
 * @tparam T A `tuple_like` type.
 * @tparam V Value type to prepend (decayed/unwrapped via `std::unwrap_ref_decay_t`).
 * @param t Source tuple-like object.
 * @param v Value to insert at index `0`.
 * @return `tuple` of size `size_v<T> + 1` with `v` followed by the elements of `t`.
 */
template <tuple_like T, class V>
raze_nodiscard constexpr auto push_front(T&& t, V&& v) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<std::unwrap_ref_decay_t<V>, element_t<Is, T>...>(
            [&] (auto i) -> decltype(auto) {
                if constexpr (i.value == 0)
                    return std::forward<V>(v);
                else
                    return __adl_get<i.value - 1>(std::forward<T>(t));
            });
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Appends `v` to the back of `t` in a single flat pass.
 * @tparam T A `tuple_like` type.
 * @tparam V Value type to append (decayed/unwrapped via `std::unwrap_ref_decay_t`).
 * @param t Source tuple-like object.
 * @param v Value to insert at index `size_v<T>`.
 * @return `tuple` of size `size_v<T> + 1` with the elements of `t` followed by `v`.
 */
template <tuple_like T, class V>
raze_nodiscard constexpr auto push_back(T&& t, V&& v) {
    constexpr sizetype n = size_v<T>;

    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<Is, T>..., std::unwrap_ref_decay_t<V>>(
            [&] (auto i) -> decltype(auto) {
                if constexpr (i.value == n)
                    return std::forward<V>(v);
                else
                    return __adl_get<i.value>(std::forward<T>(t));
            });
    }(std::make_index_sequence<n>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Removes the first element of `t` (returns `tuple<>{}` if `t` is already empty).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing elements `[1, size_v<T>)`, or `tuple<>{}`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto pop_front(T&& t) {
    if constexpr (size_v<T> == 0)
        return tuple<>{};
    else
        return meta::drop<1>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Removes the last element of `t` (returns `tuple<>{}` if `t` is already empty).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` containing elements `[0, size_v<T> - 1)`, or `tuple<>{}`.
 */
template <tuple_like T>
raze_nodiscard constexpr auto pop_back(T&& t) {
    if constexpr (size_v<T> == 0)
        return tuple<>{};
    else
        return meta::take<size_v<T> - 1>(std::forward<T>(t));
}

template <sizetype Pos, sizetype I, tuple_like T, class V>
struct __insert_elem_type {
    using type = element_t<(I < Pos) ? I : (I - 1), T>;
};

template <sizetype Pos, tuple_like T, class V>
struct __insert_elem_type<Pos, Pos, T, V> {
    using type = std::unwrap_ref_decay_t<V>;
};

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Inserts `v` at compile-time position `Pos` in a single pass with zero intermediate tuples (move-only safe).
 * @tparam Pos Insertion index (`Pos <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @tparam V Value type to insert.
 * @param t Source tuple-like object.
 * @param v Value to insert at index `Pos`.
 * @return `tuple` of size `size_v<T> + 1`.
 */
template <sizetype Pos, tuple_like T, class V>
    requires (Pos <= size_v<T>)
raze_nodiscard constexpr auto insert_at(T&& t, V&& v) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<typename __insert_elem_type<Pos, Is, T, V>::type...>(
            [&] (auto i) -> decltype(auto) {
                if constexpr (i.value < Pos)
                    return __adl_get<i.value>(std::forward<T>(t));
                else if constexpr (i.value == Pos)
                    return std::forward<V>(v);
                else
                    return __adl_get<i.value - 1>(std::forward<T>(t));
            });
    }(std::make_index_sequence<size_v<T> + 1>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `insert_at<Pos>(t, v)` accepting an `index_t<Pos>` tag.
 * @tparam Pos Insertion index (`Pos <= size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @tparam V Value type to insert.
 * @return `tuple` of size `size_v<T> + 1`.
 */
template <sizetype Pos, tuple_like T, class V>
    requires (Pos <= size_v<T>)
raze_nodiscard constexpr auto insert_at(T&& t, index_t<Pos>, V&& v) {
    return meta::insert_at<Pos>(std::forward<T>(t), std::forward<V>(v));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Erases the element at compile-time position `Pos` in a single pass with zero intermediate tuples (move-only safe).
 * @tparam Pos Index of the element to erase (`Pos < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of size `size_v<T> - 1`.
 */
template <sizetype Pos, tuple_like T>
    requires (Pos < size_v<T>)
raze_nodiscard constexpr auto erase_at(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<(Is < Pos) ? Is : (Is + 1), T>...>(
            [&] (auto i) -> decltype(auto) {
                constexpr sizetype src = (i.value < Pos) ? i.value : (i.value + 1);
                return __adl_get<src>(std::forward<T>(t));
            });
    }(std::make_index_sequence<size_v<T> - 1>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `erase_at<Pos>(t)` accepting an `index_t<Pos>` tag.
 * @tparam Pos Index of the element to erase (`Pos < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of size `size_v<T> - 1`.
 */
template <sizetype Pos, tuple_like T>
    requires (Pos < size_v<T>)
raze_nodiscard constexpr auto erase_at(T&& t, index_t<Pos>) {
    return meta::erase_at<Pos>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Replaces the element at compile-time position `Pos` with `v` (allowing the element type to change).
 * @tparam Pos Replacement index (`Pos < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @tparam V Replacement value type.
 * @param t Source tuple-like object.
 * @param v Replacement value placed at index `Pos`.
 * @return `tuple` of the same size as `t` with slot `Pos` replaced by `v`.
 */
template <sizetype Pos, tuple_like T, class V>
    requires (Pos < size_v<T>)
raze_nodiscard constexpr auto replace_at(T&& t, V&& v) {
    return __build_sequenced<size_v<T>>([&] (auto i) -> decltype(auto) {
        if constexpr (i.value == Pos)
            return std::forward<V>(v);
        else
            return __adl_get<i.value>(std::forward<T>(t));
    });
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `replace_at<Pos>(t, v)` accepting an `index_t<Pos>` tag.
 * @tparam Pos Replacement index (`Pos < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @tparam V Replacement value type.
 * @return `tuple` with slot `Pos` replaced by `v`.
 */
template <sizetype Pos, tuple_like T, class V>
    requires (Pos < size_v<T>)
raze_nodiscard constexpr auto replace_at(T&& t, index_t<Pos>, V&& v) {
    return meta::replace_at<Pos>(std::forward<T>(t), std::forward<V>(v));
}

consteval sizetype __swap_index(sizetype k, sizetype i, sizetype j) noexcept {
    return (k == i) ? j : ((k == j) ? i : k);
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Returns a new `tuple` with the elements at compile-time indices `I` and `J` swapped.
 * @tparam I First element index (`I < size_v<T>`).
 * @tparam J Second element index (`J < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` with positions `I` and `J` exchanged.
 */
template <sizetype I, sizetype J, tuple_like T>
    requires (I < size_v<T> && J < size_v<T>)
raze_nodiscard constexpr auto swap_at(T&& t) {
    return [&] <sizetype ... Ks> (std::index_sequence<Ks...>) raze_always_inline_lambda {
        return __build_typed_sequenced<element_t<__swap_index(Ks, I, J), T>...>([&] (auto k) -> decltype(auto) {
            return __adl_get<__swap_index(k.value, I, J)>(std::forward<T>(t));
        });
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `swap_at<I, J>(t)` accepting `index_t<I>` and `index_t<J>` tags.
 * @tparam I First element index (`I < size_v<T>`).
 * @tparam J Second element index (`J < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` with positions `I` and `J` exchanged.
 */
template <sizetype I, sizetype J, tuple_like T>
    requires (I < size_v<T> && J < size_v<T>)
raze_nodiscard constexpr auto swap_at(T&& t, index_t<I>, index_t<J>) {
    return meta::swap_at<I, J>(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Creates a zero-copy forwarding-reference view of `t` with positions `I` and `J` exchanged.
 * @tparam I First element index (`I < size_v<T>`).
 * @tparam J Second element index (`J < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `tuple` of references with indices `I` and `J` swapped.
 */
template <sizetype I, sizetype J, tuple_like T>
    requires (I < size_v<T> && J < size_v<T>)
raze_nodiscard raze_always_inline constexpr auto swap_at_ref(T&& t) noexcept {
    return [&] <sizetype ... Ks> (std::index_sequence<Ks...>) raze_always_inline_lambda {
        return meta::forward_as_tuple(__adl_get<__swap_index(Ks, I, J)>(std::forward<T>(t))...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_structural
 * @brief Tag-dispatched overload of `swap_at_ref<I, J>(t)` accepting `index_t<I>` and `index_t<J>` tags.
 * @tparam I First element index (`I < size_v<T>`).
 * @tparam J Second element index (`J < size_v<T>`).
 * @tparam T A `tuple_like` type.
 * @return `tuple` of references with indices `I` and `J` swapped.
 */
template <sizetype I, sizetype J, tuple_like T>
    requires (I < size_v<T> && J < size_v<T>)
raze_nodiscard raze_always_inline constexpr auto swap_at_ref(T&& t, index_t<I>, index_t<J>) noexcept {
    return meta::swap_at_ref<I, J>(std::forward<T>(t));
}

__RAZE_META_NAMESPACE_END
