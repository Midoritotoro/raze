//#pragma once 
//
//#include <raze/options/Options.h>
//
//#if defined(raze_processor_x86)
//#  include <src/raze/vx/hw/x86/compact/Compress.h>
//#endif // defined(raze_processor_x86)
//
//
//__RAZE_VX_NAMESPACE_BEGIN
//
//template <class _Options_>
//struct _Configurable_compress: raze::options::strict_elementwise_callable<_Configurable_compress, _Options_> {
//    template <simd_type _Type_, simd_mask_type _CompressMask_>
//    raze_nodiscard raze_always_inline std::pair<i32, _Type_> operator()(const _Type_& __x, const _CompressMask_& __compress_mask) const noexcept {
//        return raze::options::__dispatch_call(*this, __x, __compress_mask);
//    }
//
//    template <simd_type _Type_, simd_mask_type _CompressMask_>
//    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, const _CompressMask_& __compress_mask) noexcept {
//        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
//        using _Value_ = typename _Type_::value_type;
//        using _Abi_ = typename _Type_::abi_type;
//
//        _Type_ __result = __x;
//        auto __processed_elements = 0;
//
//        auto __chunk_op = [&] <class _Chunk, class ... _Args_> (_Chunk& __chunk, _Args_&& ... __args) raze_always_inline_lambda {
//            auto __pair = _Compress<_Abi_::isa, _Value_>()(__storage_unwrap(__chunk), __storage_unwrap<_Args_>(__args)...);
//        };
//
//        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
//            auto __condition = __options[raze::options::condition_key];
//            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});
//
//            if constexpr (_Mask_::has_alternative)
//                __result.__for_each_chunk(__chunk_op, __mask.__storage().storage(), __condition.alternative().__storage().storage());
//            else
//                __result.__for_each_chunk(__chunk_op, __mask.__storage().storage());
//        }
//        else {
//            __result.__for_each_chunk(__chunk_op);
//        }
//
//        return __result;
//    }
//
//    using callable_tag_type = _Configurable_compress;
//};
//
//__RAZE_VX_NAMESPACE_END
