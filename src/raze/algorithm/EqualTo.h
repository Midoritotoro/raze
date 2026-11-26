#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#define RAZE_DEFINE_COMPARATOR(name, op)                                            \
    template <class T>                                                              \
    struct name {                                                                   \
        static constexpr bool by_value =                                            \
            !std::is_array_v<T> && std::is_trivially_copyable_v<T> &&               \
            std::is_copy_constructible_v<T> && sizeof(T) <= sizeof(void*) * 2;      \
        using storage_type = std::conditional_t<by_value, T, const T&>;             \
        storage_type _v;                                                            \
                                                                                    \
        constexpr explicit name(const T& v) noexcept(by_value)                      \
            : _v(v)                                                                 \
        {}                                                                          \
                                                                                    \
        template <class Other>                                                      \
            requires(!vx::simd_type<std::remove_cvref_t<Other>> ||                  \
                std::same_as<typename std::remove_cvref_t<Other>::value_type, T>)   \
        raze_always_inline constexpr auto operator()(                               \
            const Other& x) const                                                   \
            noexcept(noexcept(x op _v))                                             \
        {                                                                           \
            return x op _v;                                                         \
        }                                                                           \
    }

/**
 * @ingroup raze_algo
 * @brief Transparent unary comparison functors capturing a bound right-hand value `_v`.
 *
 * Used internally by value-based algorithms (such as `replace`, `find`, `count`) to delegate
 * to their predicate counterparts (`replace_if`, `find_if`, etc.) while preserving vectorizability
 * when `_v` matches the SIMD element type without lossy narrowing.
 */
///@{
RAZE_DEFINE_COMPARATOR(equal_to, == );       ///< Evaluates `x == _v`.
RAZE_DEFINE_COMPARATOR(not_equal_to, != );   ///< Evaluates `x != _v`.
RAZE_DEFINE_COMPARATOR(less, < );            ///< Evaluates `x < _v`.
RAZE_DEFINE_COMPARATOR(greater, > );         ///< Evaluates `x > _v`.
RAZE_DEFINE_COMPARATOR(less_equal, <= );     ///< Evaluates `x <= _v`.
RAZE_DEFINE_COMPARATOR(greater_equal, >= );  ///< Evaluates `x >= _v`.
///@}

#undef RAZE_DEFINE_COMPARATOR

template <class ElemT, class Proj, class T, class Fn>
raze_always_inline constexpr decltype(auto) with_equal_to(const T& value, Fn&& fn) {
    using DecayedElem = std::remove_cvref_t<ElemT>;
    using DecayedVal = std::decay_t<const T&>;

    if constexpr (std::same_as<std::remove_cvref_t<Proj>, std::identity> &&
                  vx::arithmetic_type<DecayedElem> &&
                  std::is_arithmetic_v<DecayedVal> &&
                  !std::same_as<DecayedElem, DecayedVal>)
    {
        if constexpr (std::integral<DecayedElem> && std::integral<DecayedVal> &&
                      !std::same_as<DecayedElem, bool> && !std::same_as<DecayedVal, bool> &&
                      (std::is_signed_v<DecayedElem> == std::is_signed_v<DecayedVal> ||
                       (sizeof(DecayedElem) < sizeof(DecayedVal) && std::is_signed_v<DecayedVal>)))
        {
            if (std::in_range<DecayedElem>(value))
                return fn(equal_to<DecayedElem>(static_cast<DecayedElem>(value)));
            return fn(equal_to<DecayedVal>(value));
        }
        else if constexpr (std::floating_point<DecayedElem> && std::floating_point<DecayedVal>) {
            const auto narrowed = static_cast<DecayedElem>(value);
            if (static_cast<DecayedVal>(narrowed) == value)
                return fn(equal_to<DecayedElem>(narrowed));
            return fn(equal_to<DecayedVal>(value));
        }
    }

    return fn(equal_to<DecayedVal>(value));
}

__RAZE_ALGORITHM_NAMESPACE_END