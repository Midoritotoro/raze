#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/compact/CompressStore.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

struct unsafe_mode {};
constexpr inline auto unsafe = raze::options::flag(unsafe_mode{});
struct unsafe_option : raze::options::exact_option<unsafe> {};

template <class Options>
struct configurable_compress_store_t : options::conditional_callable<configurable_compress_store_t, Options, aligned_option, unsafe_option> {
    template <any_iterator_or_pointer Mem, simd_type T, simd_mask_type M>
    raze_always_inline Mem operator()(Mem it, const T& x, const M& mask) const noexcept {
        return options::dispatch_call(*this, it, x, mask);
    }

    template <any_iterator_or_pointer Mem, simd_type T, simd_mask_type M>
    static raze_always_inline Mem deferred_call(auto opts, Mem it, const T& x, const M& mask) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename T::value_type;
        using Abi = typename T::abi_type;

        static_assert(!options::complete_mask<Mask>,
            "compress_store does not support masks passed via options. "
            "The mask must be supplied as the last function argument.");

        constexpr auto is_unsafe = Options::contains(unsafe);

        x.__for_each_chunk([&] (const auto& chunk, const auto& mask_chunk) raze_always_inline_lambda {
            auto mem = std::to_address(it);

            if constexpr (Options::contains(aligned)) mem = reinterpret_cast<decltype(mem)>(compress_store_<Abi::isa, Value, is_unsafe>(mem, ustorage(chunk), ustorage(mask_chunk), aligned_policy{}));
            else mem = reinterpret_cast<decltype(mem)>(compress_store_<Abi::isa, Value, is_unsafe>(mem, ustorage(chunk), ustorage(mask_chunk)));

            traits::seek_iter(it, mem);
        }, mask.__storage().storage());

        return it;
    }
};

/**
 * @ingroup raze_vx_memory
 * @memberof raze::vx
 * @hideinitializer
 * @brief Compresses elements from a SIMD vector into contiguous memory according to a mask.
 *
 * Writes elements of `x` to the memory location starting at `it` for every lane where `mask[i]` 
 * evaluates to `false`. The target memory must be large enough to hold the compressed elements.
 *
 * ### Options
 * - `raze::vx::unsafe` — Enables a faster store operation. When provided, the trailing memory 
 *   region `[it + count_set(!mask), it + V::size())` is filled with garbage. 
 *   By default, these trailing elements are left unchanged.
 * - `raze::vx::aligned` — Uses aligned memory stores.
 *
 * ### Declarations
 * ```cpp
 * template <any_iterator_or_pointer Mem, simd_type T, simd_mask_type M>
 * Mem operator()(Mem it, const T& x, const M& mask) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `Mem` — The output iterator or pointer type.
 * - `T` — The source SIMD type. Defines the lane count, element type, and ABI.
 * - `M` — The mask type corresponding to `T`.
 *
 * ### Parameters
 * - `it` — The output iterator/pointer to the start of the destination buffer.
 * - `x` — The source SIMD vector.
 * - `mask` — The mask determining which elements to write. Elements where `mask[i]` is `false` are written.
 *
 * ### Mask semantics
 * **Not supported.** The mask is passed as a regular function argument. Modifiers 
 * (`[m]`, `[m, src]`) cannot be applied to `compress_store` via `operator[]`.
 *
 * ### Return value
 * An iterator/pointer pointing past the last written element (`it + count_set(!mask)`).
*/
constexpr inline auto compress_store = options::functor<configurable_compress_store_t>;

__RAZE_VX_NAMESPACE_END
