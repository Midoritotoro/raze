#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/SlideRightMerge.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_slide_right_merge_t : options::conditional_callable<configurable_slide_right_merge_t, Options> {
    template <simd_type V, sizetype Elements>
    raze_nodiscard raze_always_inline V operator()(const V& x,
        const V& y, std::integral_constant<sizetype, Elements> i) const noexcept
    {
        return options::dispatch_call(*this, x, y, i);
    }

    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, const V& y, i32 i) const noexcept {
        return options::dispatch_call(*this, x, y, i);
    }

    template <simd_type V, sizetype Elements>
    static raze_always_inline auto deferred_call(auto opts, const V& x,
        const V& y, std::integral_constant<sizetype, Elements> i) noexcept 
    {
        return slide_right_merge_(x, y, i);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y, i32 i) noexcept {
        return slide_right_merge_(x, y, i);
    }
};

/**
 * @ingroup raze_vx_shuffle
 * @memberof raze::vx
 * @hideinitializer
 * @brief Shifts the elements of a SIMD vector to the right, filling the leftmost lanes with elements from another vector.
 *
 * Shifts the elements of `x` to the right by `i` positions. The rightmost elements are discarded,
 * and the leftmost `i` lanes are filled with the last `i` elements of `y`.
 * The shift amount can be passed either as a runtime integer or a compile-time `std::integral_constant`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Compile-time shift
 * template <simd_type V, sizetype Elements>
 * V operator()(const V& x, const V& y, std::integral_constant<sizetype, Elements> i) const noexcept;
 *
 * // (2) Runtime shift
 * template <simd_type V>
 * V operator()(const V& x, const V& y, i32 i) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 * - `Elements` — The compile-time shift amount.
 *
 * ### Parameters
 * - `x` — The primary input vector (provides the right/upper elements).
 * - `y` — The secondary input vector (provides the fill elements for the left).
 * - `i` — The number of positions to shift right.
 *
 * ### Mask semantics
 * **Not supported.**  
 *
 * ### Return value
 * A vector of type `V` with `r[j] == x[j - i]` if `j >= i`, and `r[j] == y[V::size() - i + j]` otherwise.
*/
constexpr inline auto slide_right_merge = raze::options::functor<configurable_slide_right_merge_t>;

__RAZE_VX_NAMESPACE_END
