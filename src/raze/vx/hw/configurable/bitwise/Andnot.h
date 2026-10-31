#pragma once 

#include <src/raze/vx/hw/configurable/bitwise/NotAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_andnot_t : options::conditional_callable<configurable_andnot_t, Options> {
    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x, const T& y) const noexcept {
        return options::dispatch_call(*this, x, y);
    }

    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x, typename T::value_type y) const noexcept {
        return options::dispatch_call(*this, x, T(y));
    }
        
    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(typename T::value_type x, const T& y) const noexcept {
        return options::dispatch_call(*this, T(x), y);
    }

    template <simd_or_mask_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x, const T& y) noexcept {
        return bit_notand[opts](y, x);
    }
};

/**
 * @ingroup raze_vx_bitwise
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the element-wise bitwise AND NOT of two SIMD vectors or masks.
 *
 * Returns a value of type `T` where each lane is the bitwise AND of `x` and the bitwise NOT of `y`
 * (i.e., `x[i] & ~y[i]`).
 * This operation is overloaded for both standard SIMD vectors (`simd`) and masks (`simd_mask`).
 *
 * ### Declarations
 * ```cpp
 * // (1) Type-Type
 * template <simd_or_mask_type T>
 * T operator()(const T& x, const T& y) const noexcept;
 *
 * // (2) Type-Scalar
 * template <simd_or_mask_type T>
 * T operator()(const T& x, typename T::value_type y) const noexcept;
 *
 * // (3) Scalar-Type
 * template <simd_or_mask_type T>
 * T operator()(typename T::value_type x, const T& y) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `T` — The target SIMD vector or mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The left operand (vector/mask or scalar).
 * - `y` — The right operand (vector/mask or scalar).
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A value of type `T` with `r[i] == x[i] & ~y[i]` for every selected lane.
*/
constexpr inline auto bit_andnot = options::functor<configurable_andnot_t>;

__RAZE_VX_NAMESPACE_END
