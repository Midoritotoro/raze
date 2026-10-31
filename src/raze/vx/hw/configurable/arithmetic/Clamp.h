#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_clamp_t : options::conditional_callable<configurable_clamp_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& v, const V& low, const V& high) const noexcept {
        return options::dispatch_call(*this, v, low, high);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& v, const V& low, const V& high) noexcept {
        return vmin[opts](vmax[opts](v, low), high);
    }
};

/**
 * @ingroup raze_vx_arith
 * @memberof raze::vx
 * @hideinitializer
 * @brief Clamps every lane of a SIMD vector between a lower and upper bound.
 *
 * Returns a vector of type `V` where each lane is constrained to the range `[low[i], high[i]]`.
 * Evaluates `min(max(v, low), high)` element-wise.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * V operator()(const V& v, const V& low, const V& high) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `v`    — The input vector to clamp.
 * - `low`  — The lower bound vector.
 * - `high` — The upper bound vector.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics :
 *
 * ### Return value
 * A vector of type `V` with `r[i] = min(max(v[i], low[i]), high[i])` for every selected lane.
*/
constexpr inline auto clamp = options::functor<configurable_clamp_t>;

__RAZE_VX_NAMESPACE_END
