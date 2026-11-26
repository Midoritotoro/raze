#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/traits/FunctionPass.h>
#include <raze/vx/Abi.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class It>
using simd_value_t = vx::simd<std::iter_value_t<It>, vx::default_abi>;

template <class It>
using simd_mask_value_t = vx::simd_mask<std::iter_value_t<It>, vx::default_abi>;

 /**
  * @brief Checks if decayed type `T` is supported as a SIMD element type (arithmetic except `bool`,
  *        pointers, or `std::nullptr_t`).
*/
template <class T>
concept vectorizable_value_type = traits::is_vector_type_supported_v<std::decay_t<T>>;

/**
 * @brief Checks if unary predicate `Pred` cannot be vectorized (either not invocable with `simd_value_t<It>`
 *        or returns a scalar `bool`-convertible result instead of a `simd_mask`).
*/
template <class Pred, class It>
concept always_scalar_unary = vectorizable_value_type<std::iter_value_t<It>> && (
    !std::invocable<std::remove_cvref_t<Pred>, simd_value_t<It>> ||
    requires { { std::invoke(std::declval<std::remove_cvref_t<Pred>>(), std::declval<simd_value_t<It>>()) } -> std::convertible_to<bool>; });

/**
 * @brief Checks if binary predicate `Pred` cannot be vectorized.
 * @see always_scalar_unary
*/
template <class Pred, class It1, class It2 = It1>
concept always_scalar_binary = vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && (
    !std::invocable<std::remove_cvref_t<Pred>, simd_value_t<It1>, simd_value_t<It2>> ||
        requires { { std::invoke(std::declval<std::remove_cvref_t<Pred>>(), 
            std::declval<simd_value_t<It1>>(), std::declval<simd_value_t<It2>>()) } -> std::convertible_to<bool>; });


/**
 * @brief Checks if unary predicate `Pred` is invocable with `simd_value_t<It>` and yields a
 *        `simd_mask_value_t<It>` (not implicitly convertible to `bool`).
*/
template <class Pred, class It>
concept vectorizable_unary_predicate = !always_scalar_unary<Pred, It> && 
    vectorizable_value_type<std::iter_value_t<It>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<Pred>>>(), std::declval<simd_value_t<It>>()) }
            -> std::convertible_to<simd_mask_value_t<It>>;
};


/**
 * @brief Checks if binary predicate `Pred` is invocable with two `simd_value_t` arguments and
 *        yields a `simd_mask_value_t<It1>`.
*/
template <class Pred, class It1, class It2 = It1>
concept vectorizable_binary_predicate = !always_scalar_binary<Pred, It1, It2> && 
    vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<Pred>>>(), 
            std::declval<simd_value_t<It1>>(),
            std::declval<simd_value_t<It2>>()) } -> std::convertible_to<simd_mask_value_t<It1>>;
};


/**
 * @brief Checks if unary callable `F` can be invoked with `simd_value_t<It>&`.
*/
template <class F, class It>
concept vectorizable_unary_function = vectorizable_value_type<std::iter_value_t<It>> && requires {
    { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<F>>>(), std::declval<simd_value_t<It>&>()) };
};

/**
 * @brief Checks if binary callable `F` can be invoked with two `simd_value_t` references of matching element type.
*/
template <class F, class It1, class It2 = It1>
concept vectorizable_binary_function = vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && std::same_as<std::iter_value_t<It1>, std::iter_value_t<It2>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<F>>>(),
            std::declval<simd_value_t<It1>&>(), std::declval<simd_value_t<It2>&>()) };
};

/**
 * @brief Checks if projection `Proj` can be lowered in vectorized loops (currently requires `std::identity`).
*/
template <class Proj, class It>
concept vectorizable_projection = std::is_same_v<std::remove_cvref_t<traits::function_unwrapped<Proj>>, std::identity>;

template <class Comp, class T = void>
struct is_std_equal : std::false_type {};

template <>
struct is_std_equal<std::equal_to<void>, void> : std::true_type {};

template <class T>
struct is_std_equal<std::equal_to<void>, T> : std::true_type {};

template <class T>
struct is_std_equal<std::equal_to<T>, T> : std::true_type {};

template <class T>
struct is_std_equal<std::ranges::equal_to, T> : std::true_type {};

template <class Comp, class T = void>
concept is_equal_comparator = is_std_equal<std::remove_cvref_t<traits::function_unwrapped<Comp>>, T>::value;

template <class Comp, class T = void>
struct is_std_less : std::false_type {};

template <>
struct is_std_less<std::less<void>, void> : std::true_type {};

template <class T>
struct is_std_less<std::less<void>, T> : std::true_type {};

template <class T>
struct is_std_less<std::less<T>, T> : std::true_type {};

template <class T>
struct is_std_less<std::ranges::less, T> : std::true_type {};

template <class Comp, class T = void>
struct is_std_greater : std::false_type {};

template <>
struct is_std_greater<std::greater<void>, void> : std::true_type {};

template <class T>
struct is_std_greater<std::greater<void>, T> : std::true_type {};

template <class T>
struct is_std_greater<std::greater<T>, T> : std::true_type {};

template <class T>
struct is_std_greater<std::ranges::greater, T> : std::true_type {};

template <class Comp, class T = void>
concept is_less_comparator = is_std_less<std::remove_cvref_t<traits::function_unwrapped<Comp>>, T>::value;

template <class Comp, class T = void>
concept is_greater_comparator = is_std_greater<std::remove_cvref_t<traits::function_unwrapped<Comp>>, T>::value;

template <vx::simd_type Tag>
consteval bool has_native_vertical_minmax() noexcept {
    using T = typename Tag::value_type;
    constexpr auto ISA = vx::abi_t<Tag>::isa;
    constexpr auto reg_bytes = sizeof(Tag) / Tag::__chunks_count();

    if constexpr (std::floating_point<T>) return true;
    else if constexpr (reg_bytes == 64) return sizeof(T) >= 4 || vx::has_avx512bw<ISA>;
    else if constexpr (reg_bytes == 32) return sizeof(T) <= 4 ? vx::has_avx2<ISA> : vx::has_avx512vl<ISA>;
    else if constexpr (reg_bytes == 16) {
        if constexpr (std::same_as<T, i16> || std::same_as<T, u8>) return true;
        else if constexpr (sizeof(T) <= 4) return vx::has_sse41<ISA>;
        else return vx::has_avx512vl<ISA>;
    }
    else return false;
}

template <class T, class U>
concept has_common_type = requires { typename std::common_type<T, U>::type; };

template <class T, class U, bool HasCommon = has_common_type<T, U>>
struct common_type_or_void;

template <class T, class U>
struct common_type_or_void<T, U, false> { 
    using type = void;
};

template <class T, class U>
struct common_type_or_void<T, U, true> {
    using type = std::common_type_t<T, U>;
};

template <class T, class U>
using common_type_or_void_t = typename common_type_or_void<T, U>::type;

template <class It1, class It2>
concept has_common_iter_value = has_common_type<
    std::iter_value_t<It1>, std::iter_value_t<It2>>;

template <class It1, class It2>
using simd_common_value_t = vx::simd<
    common_type_or_void_t<std::iter_value_t<It1>, std::iter_value_t<It2>>,
    vx::default_abi>;

template <class It1, class It2>
using simd_common_mask_t = vx::simd_mask<
    common_type_or_void_t<std::iter_value_t<It1>, std::iter_value_t<It2>>,
    vx::default_abi>;


/**
 * @brief Variants of @ref vectorizable_binary_predicate and @ref vectorizable_binary_function
 *        operating on the `std::common_type_t` of two iterator value types.
 */
 ///@{
template <class Pred, class It1, class It2 = It1>
concept vectorizable_binary_common_predicate = 
    has_common_iter_value<It1, It2> &&
    vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && requires {
        { std::invoke(
            std::declval<traits::function_unwrapped<std::remove_cvref_t<Pred>>>(),
            std::declval<simd_common_value_t<It1, It2>&>(),
            std::declval<simd_common_value_t<It1, It2>&>()) }
            -> std::convertible_to<simd_common_mask_t<It1, It2>>;
    };

template <class F, class It1, class It2 = It1>
concept vectorizable_binary_common_function = 
    has_common_iter_value<It1, It2> &&
    vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && requires {
        { std::invoke(
            std::declval<traits::function_unwrapped<std::remove_cvref_t<F>>>(),
            std::declval<simd_common_value_t<It1, It2>&>(),
            std::declval<simd_common_value_t<It1, It2>&>()) };
    };
///@}

__RAZE_ALGORITHM_NAMESPACE_END
