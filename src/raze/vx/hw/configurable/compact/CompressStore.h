#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/compact/CompressStore.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_compress_store : raze::options::strict_elementwise_callable<_Configurable_compress_store, _Options_, aligned_option> {
    template <any_iterator_or_pointer _Mem_, simd_type _Type_, simd_mask_type _Mask_>
    raze_no_stack_protector raze_always_inline _Mem_ operator()(_Mem_ __it, const _Type_& __x, const _Mask_& __mask) const noexcept {
        return raze::options::__dispatch_call(*this, __it, __x, __mask);
    }

    template <any_iterator_or_pointer _Mem_, simd_type _Type_, simd_mask_type _MaskType_>
    static raze_no_stack_protector raze_always_inline _Mem_ deferred_call(auto __options,
        _Mem_ __it, const _Type_& __x, const _MaskType_& __mask) noexcept
    {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        static_assert(options::concepts::same_as<_Mask_, options::unknown_key>,
            "compress_store does not support masks passed via options. "
            "The mask must be supplied as the last function argument.");

        __x.__for_each_chunk([&] (const auto& __chunk, const auto& __mask_chunk) raze_always_inline_lambda {
            auto __mem = std::to_address(__it);

            if constexpr (_Options_::contains(aligned)) __mem = reinterpret_cast<decltype(__mem)>(_Compress_store<_Abi_::isa, _Value_>()(__mem, __storage_unwrap(__chunk), __storage_unwrap(__mask_chunk), __aligned_policy{}));
            else __mem = reinterpret_cast<decltype(__mem)>(_Compress_store<_Abi_::isa, _Value_>()(__mem, __storage_unwrap(__chunk), __storage_unwrap(__mask_chunk)));

            algorithm::__seek_possibly_wrapped_iterator(__it, __mem);
        }, __mask.__storage().storage());

        return __it;
    }

    using callable_tag_type = _Configurable_compress_store;
};

__RAZE_VX_NAMESPACE_END
