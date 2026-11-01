#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/SlideLeftMerge.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_slide_left_merge_t : options::conditional_callable<configurable_slide_left_merge_t, Options> {
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
        return slide_left_merge_(x, y, i);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y, i32 i) noexcept {
        return slide_left_merge_(x, y, i);
    }
};

/**
 * @ingroup raze_vx_shuffle
 * @memberof raze::vx
 * @hideinitializer
 * @brief Shifts the elements of two concatenated SIMD vectors to the left.
 *
 * Conceptually concatenates `x` and `y`, then shifts the combined sequence to the left by `i` positions.
 * The leftmost `i` elements of `x` are discarded, and the rightmost `i` lanes of the result are
 * filled with the first `i` elements of `y`.
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
 * - `x` — The first input vector (provides the lower/left elements).
 * - `y` — The second input vector (provides the upper/right elements).
 * - `i` — The number of positions to shift left.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * A vector of type `V` with `r[j] == x[j + i]` if `j + i < V::size()`, and `r[j] == y[j + i - V::size()]` otherwise.
*/
constexpr inline auto slide_left_merge = options::functor<configurable_slide_left_merge_t>;

__RAZE_VX_NAMESPACE_END
