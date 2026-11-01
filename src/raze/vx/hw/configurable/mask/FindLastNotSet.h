#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FindLastNotSet.h>
#  include <src/raze/vx/hw/x86/mask/operations/AllOf.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_find_last_not_set_t: options::conditional_callable<configurable_find_last_not_set_t, Options, not_null_option> {
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

        auto index = 0;
        constexpr auto is_unsafe = Options::contains(not_null) && M::__chunks_count() == 1;

        auto chunk_op = [&] <class Chunk, class ... Args> (const Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            auto r = find_last_not_set_<Abi::isa, Chunk::size, Value, is_unsafe>(ustorage(chunk), ustorage<Args>(args)...);
            index += r;
            return r == Chunk::size;
        };

        if constexpr (options::complete_mask<Mask>) {
            static_assert(!Mask::has_alternative, "Not supported. ");
            auto condition = opts[options::condition_key];
            x.__for_each_chunk_all_of_reverse(chunk_op, condition.mask().__storage().storage());
        }
        else {
            x.__for_each_chunk_all_of_reverse(chunk_op);
        }

        return index;
    }
};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Finds the reverse index of the highest unset lane in a SIMD mask.
 *
 * Scans the input mask `x` from the highest lane downwards and returns the reverse index
 * (where `0` corresponds to the highest lane, `1` to the next, and so on) of the first lane
 * that is `false`. If all lanes are `true`, returns `M::size()`.
 * Also accepts `algorithm::tail_mask_type` by delegating to its underlying mask.
 *
 * ### Declarations
 * ```cpp
 * template <simd_mask_type M>
 * i32 operator()(const M& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `M` — The source SIMD mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input mask to inspect.
 *
 * ### Options
 * - `raze::vx::not_null` — Assumes there is at least one unset lane.
 *   This skips the internal emptiness check and uses a faster, unsafe implementation.
 *   If applied to a mask where all lanes are `true`, behavior is undefined.
 *
 * ### Mask semantics
 * Supports **only** the maskz form (`[m]`). If a control mask is provided, only lanes where `m[i]` is `true`
 * are considered during the search.
 *
 * **Merge semantics (`[m, src]`) are not supported**  
 *
 * ### Return value
 * The reverse 0-based index of the highest active `false` lane, or `M::size()` if no such lane exists.
*/
constexpr inline auto find_last_not_set = options::functor<configurable_find_last_not_set_t>;

__RAZE_VX_NAMESPACE_END
