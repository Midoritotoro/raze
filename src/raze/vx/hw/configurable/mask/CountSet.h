#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/CountSet.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_count_set_t: options::conditional_callable<configurable_count_set_t, Options> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline i32 operator()(const M& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        auto count = 0;

        auto chunk_op = [&] <class Chunk, class ... Args> (const Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            count += count_set_<Abi::isa, Chunk::size, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            static_assert(!Mask::has_alternative, "Not supported. ");
            auto condition = opts[options::condition_key];
            x.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        else {
            x.__for_each_chunk(chunk_op);
        }

        return count;
    }
};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Counts the number of set lanes in a SIMD mask.
 *
 * Returns the number of `true` values in the input mask `x`.
 * Also accepts `algorithm::tail_mask_type` by delegating to its underlying mask.
 *
 * ### Declarations
 * ```cpp
 * // (1) Standard mask
 * template <simd_mask_type M>
 * i32 operator()(const M& x) const noexcept;
 *
 * // (2) Tail mask
 * auto operator()(algorithm::tail_mask_type auto const& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `M` — The source SIMD mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input mask to inspect.
 *
 * ### Mask semantics
 * Supports **only** the maskz form (`[m]`). If a mask is provided, only the lanes where `m[i]` is `true`
 * are considered.
 *
 * **Merge semantics (`[m, src]`) are not supported**  
 *
 * ### Return value
 * An `i32` representing the number of active lanes that are `true`.
*/
constexpr inline auto count_set = options::functor<configurable_count_set_t>;

__RAZE_VX_NAMESPACE_END
