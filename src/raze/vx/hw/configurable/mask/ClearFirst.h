#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AnyOf.h>
#  include <src/raze/vx/hw/x86/mask/operations/ClearFirst.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_clear_first_t: options::conditional_callable<configurable_clear_first_t, Options, not_null_option> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline M operator()(const M& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x) noexcept {
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        constexpr auto is_unsafe = Options::contains(not_null) && M::__chunks_count() == 1;

        M r = x;
        r.__for_each_chunk_any_of([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
            if constexpr (is_unsafe) {
                chunk = clear_first_<Abi::isa, Value, Chunk::size, is_unsafe>(ustorage(chunk));
                return true;
            }
            else {
                if (any_of_<Abi::isa, Value>(ustorage(chunk))) {
                    chunk = clear_first_<Abi::isa, Value, Chunk::size, is_unsafe>(ustorage(chunk));
                    return true;
                }
                else return false;
            }
        });

        return r;
    }
};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Clears the first set lane (lowest index) in a SIMD mask.
 *
 * Scans the input mask `x` from lane `0` upwards, finds the first lane that is `true`, and
 * sets it to `false`. All subsequent lanes remain unchanged. If no lane is `true`,
 * the mask is returned unmodified. Also accepts `algorithm::tail_mask_type` by delegating
 * to its underlying mask.
 *
 * ### Declarations
 * ```cpp
 * // (1) Standard mask
 * template <simd_mask_type M>
 * M operator()(const M& x) const noexcept;
 *
 * // (2) Tail mask
 * auto operator()(algorithm::tail_mask_type auto const& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `M` — The source SIMD mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input mask to modify.
 *
 * ### Options
 * - `raze::vx::not_null` — Assumes the input mask contains at least one `true` lane.
 *   This skips the internal emptiness check and uses a faster, unsafe implementation.
 *   If applied to an empty mask, behavior is undefined.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * A mask of type `M` with the first `true` lane set to `false`.
*/
constexpr inline auto clear_first = options::functor<configurable_clear_first_t>;

__RAZE_VX_NAMESPACE_END
