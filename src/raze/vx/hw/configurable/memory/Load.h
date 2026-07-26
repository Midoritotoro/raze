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

template <simd_type _Simd_>
struct _Configurable_load {
    template <class _Options_>
    struct __load : raze::options::strict_elementwise_callable<__load, _Options_, aligned_option> {
        template <any_iterator_or_pointer _Mem_>
        raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ operator()(_Mem_ __it) const noexcept {
            return raze::options::__dispatch_call(*this, __it);
        }

        template <any_iterator_or_pointer _Mem_>
        static raze_no_stack_protector raze_always_inline auto deferred_call(auto __options, _Mem_ __it) noexcept {
            using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
            using _Value_ = typename _Simd_::value_type;
            using _Abi_ = typename _Simd_::abi_type;

            _Simd_ __x;

            auto __mem = std::to_address(__it);

            if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
                auto __condition = __options[raze::options::condition_key];
                const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

                if constexpr (_Mask_::has_alternative)
                    __x.__for_each_chunk([] <class _Chunk, class _MaskChunk, class _SourceChunk> (
                        _Chunk & __chunk, const _MaskChunk & __mchunk, const _SourceChunk& __src_chunk, auto& __memory) raze_always_inline_lambda
                {
                    if constexpr (_Options_::contains(aligned))
                        __chunk = _Mask_load<_Abi_::isa, _Value_>()(__memory, __storage_unwrap(__mchunk), __storage_unwrap(__src_chunk), __aligned_policy{});
                    else
                        __chunk = _Mask_load<_Abi_::isa, _Value_>()(__memory, __storage_unwrap(__mchunk), __storage_unwrap(__src_chunk));

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mask.__storage().storage(), __condition.alternative().__storage().storage(), __mem);
                else
                    __x.__for_each_chunk([] <class _Chunk, class _MaskChunk> (
                        _Chunk & __chunk, const _MaskChunk & __mchunk, auto& __memory) raze_always_inline_lambda
                {
                    if constexpr (_Options_::contains(aligned))
                        __chunk = _Maskz_load<_Abi_::isa, typename _Chunk::unwrapped_type, _Value_>()(__memory, __storage_unwrap(__mchunk), __aligned_policy{});
                    else
                        __chunk = _Maskz_load<_Abi_::isa, typename _Chunk::unwrapped_type, _Value_>()(__memory, __storage_unwrap(__mchunk));

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mask.__storage().storage(), __mem);
            }
            else {
                __x.__for_each_chunk([] <class _Chunk> (_Chunk & __chunk, auto& __memory) raze_always_inline_lambda {
                    if constexpr (_Options_::contains(aligned)) __chunk = _Load<_Abi_::isa, typename _Chunk::unwrapped_type>()(__memory, __aligned_policy{});
                    else __chunk = _Load<_Abi_::isa, typename _Chunk::unwrapped_type>()(__memory);

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mem);
            }

            return __x;
        }

        using callable_tag_type = __load;
    };
};

template <simd_type _Simd_> constexpr inline auto __load = raze::options::functor<_Configurable_load<_Simd_>::template __load>;

__RAZE_VX_NAMESPACE_END
