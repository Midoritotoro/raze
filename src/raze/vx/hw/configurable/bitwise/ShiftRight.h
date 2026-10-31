#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/RightShift.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_shr_t: options::conditional_callable<configurable_shr_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, u32 shift) const noexcept {
        return options::dispatch_call(*this, x, shift);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, u32 shift) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Abi = typename V::abi_type;
        using Value = typename V::value_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&&... args) raze_always_inline_lambda {
            chunk = right_shift_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        V r = x;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, shift, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, shift, condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, shift);
        }

        return r;
    }
};

/**
 * @ingroup raze_vx_bitwise
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the element-wise bitwise right shift of a SIMD vector.
 *
 * Shifts every lane right by `shift` bits: `out[i] = x[i] >> shift`.
 *
 * **Semantics:**
 * - Unsigned `value_type`: logical shift.
 * - Signed `value_type`: arithmetic shift.
 * - If `shift >= bit_width(value_type)`, the result is zero (unsigned) or
 *   sign-extended (signed).
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * V operator()(const V& x, u32 shift) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input vector.
 * - `shift` — Per-lane shift amount (uniform across all lanes).
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A vector of type `V` with `r[i] == x[i] >> shift` for every selected lane.
*/
constexpr inline auto bit_shr = options::functor<configurable_shr_t>;

__RAZE_VX_NAMESPACE_END
