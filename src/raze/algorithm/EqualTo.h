#pragma once

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#define RAZE_DEFINE_COMPARATOR(name, op)                  \
    template <class T>                                         \
    struct name {                                                 \
        T _v;                                                  \
                                                                    \
        constexpr explicit name(T v) noexcept              \
            : _v(std::move(v))                                    \
        {}                                                          \
                                                                    \
        template <class Other>                                    \
        raze_always_inline constexpr auto operator()(               \
            const Other& x) const                               \
            noexcept(noexcept(x op _v))                   \
        {                                                           \
            return x op _v;                               \
        }                                                           \
    }

/**
 * @ingroup raze_algo
 * @brief Transparent unary comparison functors capturing a bound right-hand value `_v`.
 *
 * Used internally by value-based algorithms (such as `replace`, `find`, `count`) to delegate
 * to their predicate counterparts (`replace_if`, `find_if`, etc.) while preserving vectorizability
 * when `_v` is a scalar compatible with `raze::vx::simd`.
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

__RAZE_ALGORITHM_NAMESPACE_END