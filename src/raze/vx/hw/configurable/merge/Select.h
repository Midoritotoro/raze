#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/merge/Select.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_select_t: options::conditional_callable<configurable_select_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_mask_type M, simd_type V>
    raze_nodiscard raze_always_inline V operator()(const M& mask, const V& if_true, const V& if_false) const noexcept
        requires(Options::size() == 0 && std::same_as<M, typename V::mask_type>)
    {
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = if_true;
        r.__for_each_chunk([] <class Chunk, class AltChunk, class MaskChunk> (
            Chunk& chunk, const AltChunk& alt_chunk, const MaskChunk& mask_chunk) raze_always_inline_lambda
        {
            chunk = select_<Abi::isa, Value>(ustorage(chunk), ustorage(alt_chunk), ustorage(mask_chunk));
        }, if_false.__storage().storage(), mask.__storage().storage());
        return r;
    }

    template <simd_mask_type M, simd_type V>
    raze_nodiscard raze_always_inline V operator()(const M& mask, const V& if_true) const noexcept
        requires(Options::size() == 0 && std::same_as<M, typename V::mask_type>)
    {
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = if_true;
        r.__for_each_chunk([] <class Chunk, class MaskChunk> (
            Chunk& chunk, const MaskChunk& mask_chunk) raze_always_inline_lambda
        {
            chunk = select_<Abi::isa, Value>(ustorage(chunk), ustorage(mask_chunk));
        }, mask.__storage().storage());
        return r;
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(const auto& opts, const V& x) noexcept {
        using Mask = options::fetch_t<raze::options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = select_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            const auto& condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, condition.alternative().__storage().storage(), condition.mask().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        
        return r;
    }
};

/**
 * @ingroup raze_vx_merge
 * @memberof raze::vx
 * @hideinitializer
 * @brief Selects elements from a SIMD vector based on a mask.
 *
 * Returns a vector of type `V` where each lane is preserved from `x` if the mask is `true`,
 * and replaced with zero or an alternative source `src[i]` if `false`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * V operator()(const V& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The source vector.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics  
 *
 * ### Return value
 * A vector of type `V` containing the selected elements.
*/
constexpr inline auto select = options::functor<configurable_select_t>;

__RAZE_VX_NAMESPACE_END
