#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_swap_adjacent_t : options::conditional_callable<configurable_swap_adjacent_t, Options> {
    template <simd_type V, sizetype GroupSize>
    raze_nodiscard raze_always_inline V operator()(const V& x, std::integral_constant<sizetype, GroupSize> i) const noexcept {
        return options::dispatch_call(*this, x, i);
    }

    template <simd_type V, sizetype GroupSize>
    static raze_always_inline auto deferred_call(auto opts,
        const V& x, std::integral_constant<sizetype, GroupSize> i) noexcept
    {
        return shuffle_(x, make_swap_adjacent_pattern<V, GroupSize>{});
    }
};

/**
 * @ingroup raze_vx_shuffle
 * @memberof raze::vx
 * @hideinitializer
 * @brief Swaps adjacent blocks of elements within a SIMD vector.
 *
 * Partitions the input vector `x` into contiguous blocks of size `2 * GroupSize` and swaps
 * the two halves within each block. If the vector size is not a multiple of `2 * GroupSize`,
 * the last block is truncated, but the swap logic is applied as if the pattern wraps around.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V, sizetype GroupSize>
 * V operator()(const V& x, std::integral_constant<sizetype, GroupSize> i) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 * - `GroupSize` — The size of each sub-block to swap. Must be greater than `0`.
 *
 * ### Parameters
 * - `x` — The input vector.
 * - `i` — A compile-time tag providing the `GroupSize`.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * A vector of type `V` with adjacent blocks of size `GroupSize` swapped.
*/
constexpr inline auto swap_adjacent = options::functor<configurable_swap_adjacent_t>;

__RAZE_VX_NAMESPACE_END
