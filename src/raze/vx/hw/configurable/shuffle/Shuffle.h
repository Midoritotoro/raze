#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_shuffle_t: raze::options::conditional_callable<configurable_shuffle_t, Options> {
    template <class Pattern>
    raze_nodiscard raze_always_inline pattern_vector_t<Pattern> operator()(
        const pattern_vector_t<Pattern>& x, Pattern p) const noexcept
    {
        return options::dispatch_call(*this, x, p);
    }

    template <simd_type V, index_simd_type Index>
    raze_nodiscard raze_always_inline V operator()(const V& x, const Index& idx) const noexcept
        requires(index_type_for<Index, V>) 
    {
        return options::dispatch_call(*this, x, idx);
    }

    template <class Pattern>
    static raze_always_inline auto deferred_call(auto opts,
        const pattern_vector_t<Pattern>& x, Pattern p) noexcept
    {
        return shuffle_(x, p);
    }

    template <simd_type V, index_simd_type Index>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const Index& idx) noexcept {
        return shuffle_(x, idx);
    }
};

/**
 * @ingroup raze_vx_shuffle
 * @memberof raze::vx
 * @hideinitializer
 * @brief Permutes the elements of a SIMD vector.
 *
 * Rearranges the lanes of `x` according to a compile-time `shuffle_pattern` or a runtime
 * index vector.
 *
 * ### Declarations
 * ```cpp
 * // (1) Compile-time pattern
 * template <class Pattern>
 * pattern_vector_t<Pattern> operator()(const pattern_vector_t<Pattern>& x, Pattern p) const noexcept;
 *
 * // (2) Runtime indices
 * template <simd_type V, index_simd_type Index>
 * V operator()(const V& x, const Index& idx) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `Pattern` — A `shuffle_pattern` type defining the permutation.
 * - `V` — The target SIMD type (for runtime indices).
 * - `Index` — The SIMD type containing the runtime indices.
 *
 * ### Parameters
 * - `x` — The input vector to permute.
 * - `p` — The compile-time pattern object.
 * - `idx` — The runtime index vector. Each lane must contain a valid index within `[0, V::size())`.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * A vector of type `V` containing the permuted elements.
*/
constexpr inline auto shuffle = options::functor<configurable_shuffle_t>;

__RAZE_VX_NAMESPACE_END
