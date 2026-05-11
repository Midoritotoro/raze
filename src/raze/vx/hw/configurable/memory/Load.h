#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/Load.h>
#  include <src/raze/vx/hw/x86/memory/MaskLoad.h>
#  include <src/raze/vx/hw/x86/memory/MaskzLoad.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_load : raze::options::strict_elementwise_callable<_Configurable_load, _Options_, aligned_option> {
    template <simd_type _Simd_, any_iterator_or_pointer _Mem_>
    raze_nodiscard raze_always_inline _Simd_ operator()(raze::options::as<_Simd_> __type, _Mem_ __it) const noexcept {
        return raze::options::__dispatch_call(*this, __type, __it);
    }

    template <simd_type _Simd_, any_iterator_or_pointer _Mem_>
    static raze_always_inline auto deferred_call(auto __options, raze::options::as<_Simd_>, _Mem_ __it) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Simd_::value_type;
        using _Abi_ = typename _Simd_::abi_type;

        _Simd_ __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __x.__for_each_chunk([&] <class _Chunk, class _MaskChunk, class _SourceChunk> (
                    _Chunk& __chunk, const _MaskChunk& __mchunk, const _SourceChunk& __src_chunk) raze_always_inline_lambda 
                {
                    auto __chunk_data = __storage_unwrap(__chunk);
                    auto __mem = std::to_address(__it);
                    __chunk = _Mask_load<_Abi_::isa, _Value_>()(__mem, __storage_unwrap(__mchunk), __storage_unwrap(__src_chunk));
                    algorithm::__seek_possibly_wrapped_iterator(__it, algorithm::__bytes_pointer_offset(__mem, sizeof(_Value_) * _Chunk::size));
                }, __mask.__storage().storage(), __condition.alternative().__storage().storage());
            else
                __x.__for_each_chunk([&] <class _Chunk, class _MaskChunk> (
                    _Chunk& __chunk, const _MaskChunk& __mchunk) raze_always_inline_lambda 
                {
                    auto __chunk_data = __storage_unwrap(__chunk);
                    auto __mem = std::to_address(__it);
                    __chunk = _Maskz_load<_Abi_::isa, typename _Chunk::unwrapped_type, _Value_>()(__mem, __storage_unwrap(__mchunk));
                    algorithm::__seek_possibly_wrapped_iterator(__it, algorithm::__bytes_pointer_offset(__mem, sizeof(_Value_) * _Chunk::size));
                }, __mask.__storage().storage());
        }
        else {
            __x.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
                auto __chunk_data = __storage_unwrap(__chunk);
                auto __mem = std::to_address(__it);
                __chunk = _Load<_Abi_::isa, decltype(__chunk_data)>()(__mem);
                algorithm::__seek_possibly_wrapped_iterator(__it, algorithm::__bytes_pointer_offset(__mem, sizeof(_Value_) * _Chunk::size));
            });
        }

        return __x;
    }

    using callable_tag_type = _Configurable_load;
};

__RAZE_VX_NAMESPACE_END
