#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/traits/FunctionPass.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class It>
using simd_value_t = vx::simd<std::iter_value_t<It>, vx::default_abi>;

template <class It>
using simd_mask_value_t = vx::simd_mask<std::iter_value_t<It>, vx::default_abi>;

template <class T>
concept vectorizable_value_type = traits::is_vector_type_supported_v<std::decay_t<T>>;

template <class Pred, class It>
concept always_scalar_unary = vectorizable_value_type<std::iter_value_t<It>> && (
    !std::invocable<std::remove_cvref_t<Pred>, simd_value_t<It>> ||
    requires { { std::invoke(std::declval<std::remove_cvref_t<Pred>>(), std::declval<simd_value_t<It>>()) } -> std::convertible_to<bool>; });

template <class Pred, class It1, class It2 = It1>
concept always_scalar_binary = vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && (
    !std::invocable<std::remove_cvref_t<Pred>, simd_value_t<It1>, simd_value_t<It2>> ||
        requires { { std::invoke(std::declval<std::remove_cvref_t<Pred>>(), 
            std::declval<simd_value_t<It1>>(), std::declval<simd_value_t<It2>>()) } -> std::convertible_to<bool>; });

template <class Pred, class It>
concept vectorizable_unary_predicate = !always_scalar_unary<Pred, It> && 
    vectorizable_value_type<std::iter_value_t<It>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<Pred>>>(), std::declval<simd_value_t<It>>()) }
            -> std::convertible_to<simd_mask_value_t<It>>;
};

template <class Pred, class It1, class It2 = It1>
concept vectorizable_binary_predicate = !always_scalar_binary<Pred, It1, It2> && 
    vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<Pred>>>(), 
            std::declval<simd_value_t<It1>>(),
            std::declval<simd_value_t<It2>>()) } -> std::convertible_to<simd_mask_value_t<It1>>;
};

template <class F, class It>
concept vectorizable_unary_function = vectorizable_value_type<std::iter_value_t<It>> && requires {
    { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<F>>>(), std::declval<simd_value_t<It>&>()) };
};

template <class F, class It1, class It2 = It1>
concept vectorizable_binary_function = vectorizable_value_type<std::iter_value_t<It1>> && 
    vectorizable_value_type<std::iter_value_t<It2>> && requires {
        { std::invoke(std::declval<traits::function_unwrapped<std::remove_cvref_t<F>>>(),
            std::declval<simd_value_t<It1>&>(), std::declval<simd_value_t<It2>&>()) };
};

template <class Proj, class It>
concept vectorizable_projection = std::is_same_v<std::remove_cvref_t<traits::function_unwrapped<Proj>>, std::identity>;

template <class Comp>
concept is_less_comparator = std::is_same_v<std::remove_cvref_t<traits::function_unwrapped<Comp>>, std::less<>>;

template <class Comp>
concept is_greater_comparator = std::is_same_v<std::remove_cvref_t<traits::function_unwrapped<Comp>>, std::greater<>>;

__RAZE_ALGORITHM_NAMESPACE_END
