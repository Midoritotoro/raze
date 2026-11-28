#pragma once

#include <src/raze/meta/tuple/Tuple.h>


__RAZE_META_NAMESPACE_BEGIN

template <class T>
struct __is_tuple_impl : std::false_type {};

template <class ... Ts>
struct __is_tuple_impl<tuple<Ts...>> : std::true_type {};

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Evaluates to `true` if `std::remove_cvref_t<T>` is a specialization of `raze::meta::tuple`.
 * @tparam T Type to test (cv-qualifiers and references are ignored).
 */
template <class T>
inline constexpr bool is_tuple_v = __is_tuple_impl<std::remove_cvref_t<T>>::value;

template <class T>
concept __has_tuple_size = requires {
    typename std::tuple_size<std::remove_cvref_t<T>>::type;
    { std::tuple_size<std::remove_cvref_t<T>>::value } -> std::convertible_to<sizetype>;
};

template <class T>
consteval bool __check_tuple_elements() noexcept {
    if constexpr (!__has_tuple_size<T>) {
        return false;
    }
    else {
        using decayed_t = std::remove_cvref_t<T>;

        return [] <sizetype ... Is> (std::index_sequence<Is...>) {
            return (requires (T&& t) {
                typename std::tuple_element<Is, decayed_t>::type;
                __adl_get<Is>(std::forward<T>(t));
            } && ...);
        }(std::make_index_sequence<std::tuple_size_v<decayed_t>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Concept satisfied by any type implementing the standard tuple protocol.
 *
 * Matches `raze::meta::tuple`, `std::tuple`, `std::pair`, `std::array`, `std::ranges::subrange`,
 * and any user-defined type providing valid `std::tuple_size`, `std::tuple_element`, and ADL
 * `get<I>` for every index `0 <= I < size_v<T>`.
 *
 * @tparam T Candidate type (may be cvref-qualified).
 */
template <class T>
concept tuple_like = __has_tuple_size<T> && __check_tuple_elements<T>();

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Number of elements in a `tuple_like` type `T` (ignoring cv-qualifiers and references).
 * @tparam T A `tuple_like` type.
 */
template <tuple_like T>
inline constexpr sizetype size_v = std::tuple_size_v<std::remove_cvref_t<T>>;

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Declared `I`-th element type of a `tuple_like` type `T` (`std::tuple_element_t`).
 * @tparam I Zero-based element index.
 * @tparam T A `tuple_like` type.
 */
template <sizetype I, tuple_like T>
using element_t = std::tuple_element_t<I, std::remove_cvref_t<T>>;

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Exact return type of `get<I>(std::declval<T>())` preserving cvref qualification of `T`.
 * @tparam I Zero-based element index.
 * @tparam T A `tuple_like` type (cvref-qualified).
 */
template <sizetype I, tuple_like T>
using member_t = decltype(__adl_get<I>(std::declval<T>()));

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Concept satisfied by `tuple_like` types with exactly `N` elements.
 * @tparam T Candidate type.
 * @tparam N Required element count.
 */
template <class T, sizetype N>
concept sized_tuple_like = tuple_like<T> && (size_v<T> == N);

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Concept satisfied by `tuple_like` types containing at least one element (`size_v<T> > 0`).
 * @tparam T Candidate type.
 */
template <class T>
concept non_empty_tuple_like = tuple_like<T> && (size_v<T> > 0);

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Concept satisfied when `T` and all `Us...` are `tuple_like` and share the same `size_v`.
 * @tparam T First tuple-like type.
 * @tparam Us Additional tuple-like types to compare against `T`.
 */
template <class T, class ... Us>
concept same_size_tuples = tuple_like<T> && (tuple_like<Us> && ...) && ((size_v<T> == size_v<Us>) && ...);

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Number of occurrences of exact type `Target` among the elements of `T`.
 * @tparam Target Type to count.
 * @tparam T A `tuple_like` type.
 */
template <class Target, tuple_like T>
inline constexpr sizetype count_type_v = [] <sizetype ... Is> (std::index_sequence<Is...>) {
    return __count_type_in_pack<Target, element_t<Is, T>...>();
}(std::make_index_sequence<size_v<T>>{});

/**
 * @ingroup raze_meta_tuple_traits
 * @brief `true` if `Target` appears at least once among the element types of `T`.
 * @tparam Target Type to search for.
 * @tparam T A `tuple_like` type.
 */
template <class Target, tuple_like T>
inline constexpr bool contains_type_v = (count_type_v<Target, T> > 0);

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Zero-based index of the unique occurrence of `Target` in `T`.
 *
 * Constrained on `count_type_v<Target, T> == 1`.
 *
 * @tparam Target Unique element type to locate.
 * @tparam T A `tuple_like` type containing `Target` exactly once.
 */
template <class Target, tuple_like T>
    requires (count_type_v<Target, T> == 1)
inline constexpr sizetype type_index_v = [] <sizetype ... Is> (std::index_sequence<Is...>) {
    return __index_of_unique_type<Target, element_t<Is, T>...>();
}(std::make_index_sequence<size_v<T>>{});

/**
 * @ingroup raze_meta_tuple_traits
 * @brief `true` if every element type of `T` is distinct (`true` for empty tuples).
 * @tparam T A `tuple_like` type.
 */
template <tuple_like T>
inline constexpr bool all_unique_types_v = [] <sizetype ... Is> (std::index_sequence<Is...>) {
    return ((count_type_v<element_t<Is, T>, T> == 1) && ...);
}(std::make_index_sequence<size_v<T>>{});

/**
 * @ingroup raze_meta_tuple_traits
 * @brief `true` if `T` is non-empty and all of its element types are identical.
 * @tparam T A `tuple_like` type.
 */
template <tuple_like T>
inline constexpr bool is_homogeneous_v = [] <sizetype ... Is> (std::index_sequence<Is...>) {
    return __all_same_v<element_t<Is, T>...>;
}(std::make_index_sequence<size_v<T>>{});

template <template <class ...> class Trait, tuple_like T, class Seq>
struct __apply_traits_impl;

template <template <class ...> class Trait, tuple_like T, sizetype ... Is>
struct __apply_traits_impl<Trait, T, std::index_sequence<Is...>> {
    using type = typename Trait<element_t<Is, T>...>::type;
};

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Applies a variadic metafunction `Trait<Ts...>::type` to the unpacked element types of `T`.
 * @tparam Trait Variadic trait template exposing a nested `::type`.
 * @tparam T A `tuple_like` type whose element types are unpacked into `Trait`.
 */
template <template <class ...> class Trait, tuple_like T>
using apply_traits_t = typename __apply_traits_impl<Trait, T, std::make_index_sequence<size_v<T>>>::type;

template <template <class> class Trait, tuple_like T, class Seq>
struct __map_traits_impl;

template <template <class> class Trait, tuple_like T, sizetype ... Is>
struct __map_traits_impl<Trait, T, std::index_sequence<Is...>> {
    using type = tuple<typename Trait<element_t<Is, T>>::type...>;
};

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Maps a unary metafunction `Trait<E>::type` over each element type of `T`, producing a `raze::meta::tuple`.
 * @tparam Trait Unary trait template exposing a nested `::type`.
 * @tparam T A `tuple_like` type.
 */
template <template <class> class Trait, tuple_like T>
using map_traits_t = typename __map_traits_impl<Trait, T, std::make_index_sequence<size_v<T>>>::type;

template <class T, bool IsTuple = tuple_like<T>>
struct __as_tuple_impl {
    using type = tuple<std::remove_cvref_t<T>>;
};

template <class T, class Seq>
struct __tuple_like_to_tuple;

template <class T, sizetype ... Is>
struct __tuple_like_to_tuple<T, std::index_sequence<Is...>> {
    using type = tuple<element_t<Is, T>...>;
};

template <class T>
struct __as_tuple_impl<T, true> :
    __tuple_like_to_tuple<std::remove_cvref_t<T>, std::make_index_sequence<size_v<T>>>
{};

/**
 * @ingroup raze_meta_tuple_traits
 * @brief Normalizes `T` to a `raze::meta::tuple` type: converts `tuple_like` types element-wise, or wraps scalars as `tuple<std::remove_cvref_t<T>>`.
 * @tparam T Any type (tuple-like or scalar).
 */
template <class T>
using as_tuple_t = typename __as_tuple_impl<T>::type;

__RAZE_META_NAMESPACE_END
