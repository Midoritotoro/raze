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
    raze_always_inline _Mem_ operator()(Mem it, const T& x, const M& mask) const noexcept {
        return options::dispatch_call(*this, it, x, mask);
    }

    template <any_iterator_or_pointer Mem, simd_type T, simd_mask_type M>
    static raze_always_inline _Mem_ deferred_call(auto opts, Mem it, const T& x, const M& mask) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename T::value_type;
        using Abi = typename T::abi_type;

        static_assert(!options::complete_mask<Mask>,
            "compress_store does not support masks passed via options. "
            "The mask must be supplied as the last function argument.");

        __x.__for_each_chunk([&] (const auto& __chunk, const auto& __mask_chunk) raze_always_inline_lambda {
            auto __mem = std::to_address(__it);

            if constexpr (_Options_::contains(aligned)) __mem = reinterpret_cast<decltype(__mem)>(_Compress_store<_Abi_::isa, _Value_>()(__mem, ustorage(__chunk), ustorage(__mask_chunk), __aligned_policy{}));
            else __mem = reinterpret_cast<decltype(__mem)>(_Compress_store<_Abi_::isa, _Value_>()(__mem, ustorage(__chunk), ustorage(__mask_chunk)));

            algorithm::__seek_iter(__it, __mem);
        }, __mask.__storage().storage());

        return __it;
    }
};

__RAZE_VX_NAMESPACE_END
