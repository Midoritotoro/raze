#pragma once

#include <raze/Types.h>
#include <src/raze/compatibility/Compatibility.h>
#include <type_traits>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tags
 * @struct index_t raze/meta/Tuple.h <raze/meta/Tuple.h>
 * @brief Compile-time non-negative index tag carrying the value `I` in its type.
 *
 * Used to index `raze::meta::tuple` via `operator[]` and to pass compile-time bounds,
 * counts, or permutation indices as function arguments (e.g., `extract(t, 1_c, 4_c)`).
 * Implicitly converts to both `sizetype` and `std::integral_constant<sizetype, I>`.
 *
 * @tparam I Compile-time index value.
 */
template <sizetype I>
struct index_t {
    /** @brief The compile-time index value `I`. */
    static constexpr sizetype value = I;
    /** @brief Underlying integral type (`sizetype`). */
    using value_type = sizetype;
    /** @brief Self alias for `std::integral_constant`-compatible metaprogramming. */
    using type = index_t;

    /**
     * @brief Implicitly converts the tag to its underlying `sizetype` value.
     * @return `I`.
     */
    raze_always_inline constexpr operator sizetype() const noexcept {
        return I;
    }

    /**
     * @brief Implicitly converts the tag to `std::integral_constant<sizetype, I>`.
     * @return Default-constructed `std::integral_constant<sizetype, I>{}`.
     */
    raze_always_inline constexpr operator std::integral_constant<sizetype, I>() const noexcept {
        return {};
    }
};

/**
 * @ingroup raze_meta_tags
 * @brief Variable template instance of `index_t<I>`.
 * @tparam I Compile-time index value.
 */
template <sizetype I>
inline constexpr index_t<I> index{};

/**
 * @ingroup raze_meta_tags
 * @struct type_t raze/meta/Tuple.h <raze/meta/Tuple.h>
 * @brief Empty compile-time type tag carrying `T` without instantiating an object of type `T`.
 *
 * Enables type-based indexing on `raze::meta::tuple` via `t[type_v<T>]` when `T` occurs
 * uniquely in the tuple.
 *
 * @tparam T The wrapped type.
 */
template <class T>
struct type_t {
    /** @brief The wrapped type `T`. */
    using type = T;
};

/**
 * @ingroup raze_meta_tags
 * @brief Variable template instance of `type_t<T>`.
 * @tparam T The wrapped type.
 */
template <class T>
inline constexpr type_t<T> type_v{};

inline namespace literals {

template <char ... Cs>
constexpr sizetype __parse_index() noexcept {
    static_assert(sizeof...(Cs) > 0, "Empty index literal");
    static_assert((((Cs >= '0' && Cs <= '9') || Cs == '\'') && ...),
        "Invalid digit in index literal");

    sizetype v = 0;
    (((Cs == '\'') ? (void)0 : (void)(v = v * 10 + static_cast<sizetype>(Cs - '0'))), ...);
    return v;
}

/**
 * @ingroup raze_meta_tags
 * @brief User-defined literal producing a compile-time `index_t<N>` tag.
 *
 * Parses base-10 integer literals (including C++ digit separators `'`, e.g., `0_c`, `3_c`, `1'000_c`)
 * into `index_t<N>`. Because `literals` is an `inline namespace` inside `raze::meta`, the
 * literal is available automatically with `using namespace raze::meta;` or
 * `using namespace raze::meta::literals;`.
 *
 * @tparam Cs Decimal digit characters (and optional `'` separators).
 * @return `index_t<N>{}` where `N` is the parsed decimal value.
 */
template <char ... Cs>
raze_always_inline constexpr index_t<__parse_index<Cs...>()> operator""_c() noexcept {
    return {};
}

} // namespace literals

__RAZE_META_NAMESPACE_END
