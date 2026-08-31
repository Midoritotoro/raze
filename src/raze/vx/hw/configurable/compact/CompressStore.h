#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/compact/CompressStore.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_compress_store_t : options::conditional_callable<configurable_compress_store_t, Options, aligned_option> {
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

        x.__for_each_chunk([&] (const auto& chunk, const auto& mask_chunk) raze_always_inline_lambda {
            auto mem = std::to_address(it);

            if constexpr (Options::contains(aligned)) mem = reinterpret_cast<decltype(mem)>(compress_store_<Abi::isa, Value>(mem, ustorage(chunk), ustorage(mask_chunk), aligned_policy{}));
            else mem = reinterpret_cast<decltype(mem)>(compress_store_<Abi::isa, Value>(mem, ustorage(chunk), ustorage(mask_chunk)));

            algorithm::seek_iter(it, mem);
        }, mask.__storage().storage());

        return it;
    }
};

constexpr inline auto compress_store = options::functor<configurable_compress_store_t>;

__RAZE_VX_NAMESPACE_END
