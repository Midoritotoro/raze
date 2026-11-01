#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/Load.h>
#  include <src/raze/vx/hw/x86/memory/MaskLoad.h>
#  include <src/raze/vx/hw/x86/memory/MaskzLoad.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V>
struct configurable_load_t {
    template <class Options>
    struct load : options::conditional_callable<load, Options, aligned_option, safe_option> {
        template <any_iterator_or_pointer Mem>
        raze_nodiscard raze_always_inline V operator()(Mem it) const noexcept {
            return options::dispatch_call(*this, it);
        }

        template <any_iterator_or_pointer Mem>
        static raze_always_inline auto deferred_call(auto opts, Mem it) noexcept {
            using Mask = options::fetch_t<options::condition_key, Options>;
            using Value = typename V::value_type;
            using Abi = typename V::abi_type;

            constexpr auto is_safe = Options::contains(safe);

            V x;
            auto mem = std::to_address(it);

            if constexpr (options::complete_mask<Mask>) {
                auto condition = opts[options::condition_key];

                if constexpr (Mask::has_alternative)
                    x.__for_each_chunk([] <class Chunk, class MaskChunk, class SourceChunk> (
                        Chunk& chunk, const MaskChunk& mchunk, const SourceChunk& src_chunk, auto& memory) raze_always_inline_lambda
                {
                    if constexpr (Options::contains(aligned))
                        chunk = load_<Abi::isa, Value, is_safe>(memory, ustorage(mchunk), ustorage(src_chunk), aligned_policy{});
                    else
                        chunk = load_<Abi::isa, Value, is_safe>(memory, ustorage(mchunk), ustorage(src_chunk));

                    algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
                }, condition.mask().__storage().storage(), condition.alternative().__storage().storage(), mem);
                else
                    x.__for_each_chunk([] <class Chunk, class MaskChunk> (
                        Chunk& chunk, const MaskChunk& mchunk, auto& memory) raze_always_inline_lambda
                {
                    if constexpr (Options::contains(aligned))
                        chunk = load_<Abi::isa, typename Chunk::unwrapped_type, Value, is_safe>(memory, ustorage(mchunk), aligned_policy{});
                    else
                        chunk = load_<Abi::isa, typename Chunk::unwrapped_type, Value, is_safe>(memory, ustorage(mchunk));

                    algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
                }, condition.mask().__storage().storage(), mem);
            }
            else {
                x.__for_each_chunk([] <class Chunk> (Chunk& chunk, auto& memory) raze_always_inline_lambda {
                    if constexpr (Options::contains(aligned)) chunk = load_<Abi::isa, typename Chunk::unwrapped_type>(memory, aligned_policy{});
                    else chunk = load_<Abi::isa, typename Chunk::unwrapped_type>(memory);

                    algorithm::advance_bytes(memory, sizeof(Value) * Chunk::size);
                }, mem);
            }

            return x;
        }
    };
};

/**
 * @ingroup raze_vx_memory
 * @memberof raze::vx
 * @hideinitializer
 * @brief Loads a SIMD vector from memory.
 *
 * Reads `V::size()` elements starting from the provided iterator or pointer and packs them into
 * a vector of type `V`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * constexpr inline *unspecified* load;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `it` — The source iterator or pointer to the first element to load.
 *
 * ### Options
 * - `raze::vx::aligned` — Uses aligned memory load instructions. The pointer must satisfy the alignment
 *   requirements of `V`.
 * - `raze::vx::safe` — Enables safe memory loading, preventing out-of-bounds reads at the end of mapped pages.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics :
 *
 * ### Return value
 * A vector of type `V` containing the loaded elements. In masked forms, unselected lanes are zero-filled
 * or copied from `src`.
*/
template <simd_type V> 
constexpr inline auto load = options::functor<configurable_load_t<V>::template load>;

__RAZE_VX_NAMESPACE_END
