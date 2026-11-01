#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AnyOf.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_any_of_t: options::conditional_callable<configurable_any_of_t, Options> {
    template <simd_mask_type V>
    raze_nodiscard raze_always_inline bool operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (const Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            return any_of_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            static_assert(!Mask::has_alternative, "Not supported. ");

            auto condition = opts[options::condition_key];
            return x.__for_each_chunk_any_of(chunk_op, condition.mask().__storage().storage());
        }
        else {
            return x.__for_each_chunk_any_of(chunk_op);
        }
    }
};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Checks if any active lanes of a SIMD mask are `true`.
 *
 * Returns `true` if at least one active lane in the input mask `x` evaluates to `true`.
 * If the mask is entirely `false`, the result is `false`.
 * Also accepts `algorithm::tail_mask_type` by delegating to its underlying mask.
 *
 * ### Declarations
 * ```cpp
 * // (1) Standard mask
 * template <simd_mask_type V>
 * bool operator()(const V& x) const noexcept;
 *
 * // (2) Tail mask
 * auto operator()(algorithm::tail_mask_type auto const& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The source SIMD mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input mask to inspect.
 *
 * ### Mask semantics
 * Supports **only** the maskz form (`[m]`). If a mask is provided, only the lanes where `m[i]` is `true`
 * are checked against the input mask.
 *
 * **Merge semantics (`[m, src]`) are not supported** (a `static_assert` will trigger).
 *
 * ### Return value
 * `true` if at least one active lane is `true`, `false` otherwise.
*/
constexpr inline auto any_of = options::functor<configurable_any_of_t>;

__RAZE_VX_NAMESPACE_END
