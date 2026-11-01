#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <src/raze/algorithm/AdvanceBytes.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/Store.h>
#  include <src/raze/vx/hw/x86/memory/MaskStore.h>
#  include <src/raze/vx/hw/x86/merge/Select.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_store_t : raze::options::conditional_callable<configurable_store_t, Options, aligned_option, safe_option> {
    template <any_iterator_or_pointer Mem, simd_type V>
    raze_always_inline void operator()(Mem it, const V& x) const noexcept {
        return options::dispatch_call(*this, it, x);
    }

    template <any_iterator_or_pointer Mem, simd_type V>
    static raze_always_inline auto deferred_call(auto const& opts, Mem it, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        constexpr auto is_safe = Options::contains(safe);
		auto mem = std::to_address(it);

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative)
                return x.__for_each_chunk([] <class Chunk, class MaskChunk, class SourceChunk> (
                    Chunk& chunk, const MaskChunk& mchunk, const SourceChunk& src_chunk, auto& memory) raze_always_inline_lambda
                {
                    if constexpr (Options::contains(aligned)) store_(memory, select_<Abi::isa, Value>(ustorage(chunk),
                        ustorage(src_chunk), ustorage(mchunk)), aligned_policy{});
                    else store_(memory, select_<Abi::isa, Value>(ustorage(chunk),
                        ustorage(src_chunk), ustorage(mchunk)));

                    algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
                }, mask.__storage().storage(), condition.alternative().__storage().storage(), mem);
            else
                return x.__for_each_chunk([] <class Chunk, class MaskChunk> (
                    Chunk& chunk, const MaskChunk& mchunk, auto& memory) raze_always_inline_lambda
                {
                    if constexpr (Options::contains(aligned)) store_<Abi::isa, Value, is_safe>(memory, ustorage(mchunk), ustorage(chunk), aligned_policy{});
                    else store_<Abi::isa, Value, is_safe>(memory, ustorage(mchunk), ustorage(chunk));

                    algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
                }, mask.__storage().storage(), mem);
        }
        else {
            return x.__for_each_chunk([] <class Chunk> (Chunk& chunk, auto& memory) raze_always_inline_lambda {
                if constexpr (Options::contains(aligned)) store_(memory, ustorage(chunk), aligned_policy{});
                else store_(memory, ustorage(chunk));

                algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
            }, mem);
        }
    }
};

/**
 * @ingroup raze_vx_memory
 * @memberof raze::vx
 * @hideinitializer
 * @brief Stores a SIMD vector to memory.
 *
 * Writes the elements of `x` to the memory location starting at `it`.
 *
 * ### Declarations
 * ```cpp
 * template <any_iterator_or_pointer Mem, simd_type V>
 * void operator()(Mem it, const V& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `Mem` — The output iterator or pointer type.
 * - `V` — The source SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `it` — The destination iterator or pointer.
 * - `x` — The source SIMD vector.
 *
 * ### Options
 * - `raze::vx::aligned` — Uses aligned memory store instructions. The pointer must satisfy the alignment
 *   requirements of `V`.
 * - `raze::vx::safe` — Guarantees that unselected memory lanes are entirely untouched (no reads or writes).
 *   Without this option, unselected lanes remain value-unchanged, but the memory may still be accessed
 *   (touched), which can cause faults if the memory is unmapped.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics :
 * - **Maskz (`[m]`)**: Writes `x[i]` to memory only if `m[i]` is `true`. Unselected elements in memory remain value-unchanged.
 * - **Merge (`[m, src]`)**: Writes `x[i]` to memory if `m[i]` is `true`, otherwise writes `src[i]`.
 *
 * ### Return value
 * (none)
*/
constexpr inline auto store = options::functor<configurable_store_t>;

__RAZE_VX_NAMESPACE_END
