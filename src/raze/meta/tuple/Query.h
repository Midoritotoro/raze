#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether `pred(elem)` evaluates to `true` for every element of `t` (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked on each element.
 * @return `true` if all elements satisfy `pred` (vacuously `true` for empty tuples).
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool all_of(T&& t, Pred&& pred) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return (static_cast<bool>(__invoke(pred, __adl_get<Is>(std::forward<T>(t)))) && ...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether every element of `t` is contextually convertible to `true` (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if all elements evaluate to `true` (vacuously `true` for empty tuples).
 */
template <tuple_like T>
raze_nodiscard constexpr bool all_of(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return (static_cast<bool>(__adl_get<Is>(std::forward<T>(t))) && ...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether `pred(elem)` evaluates to `true` for every element of `t` in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked from `size_v<T> - 1` down to `0`.
 * @return `true` if all elements satisfy `pred` (vacuously `true` for empty tuples).
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool all_of_reversed(T&& t, Pred&& pred) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return true;
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return (static_cast<bool>(__invoke(pred, __adl_get<n - 1 - Is>(std::forward<T>(t)))) && ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether every element of `t` is truthy, evaluating in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if all elements evaluate to `true` (vacuously `true` for empty tuples).
 */
template <tuple_like T>
raze_nodiscard constexpr bool all_of_reversed(T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return true;
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return (static_cast<bool>(__adl_get<n - 1 - Is>(std::forward<T>(t))) && ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether `pred(elem)` evaluates to `true` for at least one element of `t` (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked on each element.
 * @return `true` if any element satisfies `pred` (`false` for empty tuples).
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool any_of(T&& t, Pred&& pred) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return (static_cast<bool>(__invoke(pred, __adl_get<Is>(std::forward<T>(t)))) || ...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether at least one element of `t` is truthy (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if any element evaluates to `true` (`false` for empty tuples).
 */
template <tuple_like T>
raze_nodiscard constexpr bool any_of(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        return (static_cast<bool>(__adl_get<Is>(std::forward<T>(t))) || ...);
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether `pred(elem)` evaluates to `true` for at least one element of `t` in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked from `size_v<T> - 1` down to `0`.
 * @return `true` if any element satisfies `pred` (`false` for empty tuples).
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool any_of_reversed(T&& t, Pred&& pred) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return false;
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return (static_cast<bool>(__invoke(pred, __adl_get<n - 1 - Is>(std::forward<T>(t)))) || ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether at least one element of `t` is truthy, evaluating in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if any element evaluates to `true` (`false` for empty tuples).
 */
template <tuple_like T>
raze_nodiscard constexpr bool any_of_reversed(T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return false;
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return (static_cast<bool>(__adl_get<n - 1 - Is>(std::forward<T>(t))) || ...);
        }(std::make_index_sequence<n>{});
    }
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether no element of `t` satisfies `pred` (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked on each element.
 * @return `true` if `!any_of(t, pred)`.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool none_of(T&& t, Pred&& pred) {
    return !meta::any_of(std::forward<T>(t), std::forward<Pred>(pred));
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether no element of `t` is truthy (left-to-right, short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if `!any_of(t)`.
 */
template <tuple_like T>
raze_nodiscard constexpr bool none_of(T&& t) {
    return !meta::any_of(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether no element of `t` satisfies `pred`, evaluating in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate invoked from right to left.
 * @return `true` if `!any_of_reversed(t, pred)`.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr bool none_of_reversed(T&& t, Pred&& pred) {
    return !meta::any_of_reversed(std::forward<T>(t), std::forward<Pred>(pred));
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Checks whether no element of `t` is truthy, evaluating in reverse order (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return `true` if `!any_of_reversed(t)`.
 */
template <tuple_like T>
raze_nodiscard constexpr bool none_of_reversed(T&& t) {
    return !meta::any_of_reversed(std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Counts how many elements of `t` satisfy unary predicate `pred`.
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate applied to each element.
 * @return Number of matching elements.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr sizetype count_if(T&& t, Pred&& pred) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> sizetype {
        return ((static_cast<bool>(__invoke(pred, __adl_get<Is>(std::forward<T>(t))))
            ? sizetype{ 1 } : sizetype{ 0 }) + ... + sizetype{ 0 });
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Counts how many elements of `t` are contextually convertible to `true`.
 * @tparam T A `tuple_like` type.
 * @param t Source tuple-like object.
 * @return Number of truthy elements.
 */
template <tuple_like T>
raze_nodiscard constexpr sizetype count(T&& t) {
    return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> sizetype {
        return ((static_cast<bool>(__adl_get<Is>(std::forward<T>(t)))
            ? sizetype{ 1 } : sizetype{ 0 }) + ... + sizetype{ 0 });
    }(std::make_index_sequence<size_v<T>>{});
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Counts how many elements of `t` compare equal to `v`.
 *
 * Heterogeneous elements whose type is not equality-comparable with `Value` safely evaluate to `false`
 * rather than failing to compile.
 *
 * @tparam T A `tuple_like` type.
 * @tparam Value Target value type.
 * @param t Source tuple-like object.
 * @param v Value to compare elements against.
 * @return Number of elements comparing equal to `v`.
 */
template <tuple_like T, class Value>
raze_nodiscard constexpr sizetype count(T&& t, const Value& v) {
    return meta::count_if(std::forward<T>(t), [&] (const auto& elem) {
        if constexpr (requires { { elem == v } -> std::convertible_to<bool>; })
            return static_cast<bool>(elem == v);
        else
            return false;
    });
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Finds the zero-based index of the first element in `t` satisfying `pred` (short-circuiting).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate tested from `0` to `size_v<T> - 1`.
 * @return Index of the first matching element, or `size_v<T>` if none matches.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr sizetype find_index_if(T&& t, Pred&& pred) {
    sizetype result = size_v<T>;

    [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
        static_cast<void>(((static_cast<bool>(__invoke(pred, __adl_get<Is>(std::forward<T>(t))))
            ? (result = Is, true) : false) || ...));
    }(std::make_index_sequence<size_v<T>>{});

    return result;
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Alias for `find_index_if(t, pred)`.
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate tested from left to right.
 * @return Index of the first matching element, or `size_v<T>` if none matches.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr sizetype locate(T&& t, Pred&& pred) {
    return meta::find_index_if(std::forward<T>(t), std::forward<Pred>(pred));
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Finds the zero-based index of the last element in `t` satisfying `pred` (short-circuiting from right to left).
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate tested from `size_v<T> - 1` down to `0`.
 * @return Index of the last matching element, or `size_v<T>` if none matches.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr sizetype find_last_index_if(T&& t, Pred&& pred) {
    constexpr sizetype n = size_v<T>;
    sizetype result = n;

    if constexpr (n > 0) {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            static_cast<void>(((static_cast<bool>(__invoke(pred, __adl_get<n - 1 - Is>(std::forward<T>(t))))
                ? (result = n - 1 - Is, true) : false) || ...));
        }(std::make_index_sequence<n>{});
    }

    return result;
}

/**
 * @ingroup raze_meta_tuple_query
 * @brief Alias for `find_last_index_if(t, pred)`.
 * @tparam T A `tuple_like` type.
 * @tparam Pred Unary predicate callable.
 * @param t Source tuple-like object.
 * @param pred Predicate tested from right to left.
 * @return Index of the last matching element, or `size_v<T>` if none matches.
 */
template <tuple_like T, class Pred>
raze_nodiscard constexpr sizetype rlocate(T&& t, Pred&& pred) {
    return meta::find_last_index_if(std::forward<T>(t), std::forward<Pred>(pred));
}

__RAZE_META_NAMESPACE_END
