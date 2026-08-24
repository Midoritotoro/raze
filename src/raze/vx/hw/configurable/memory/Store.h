#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <src/raze/algorithm/AdvanceBytes.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/memory/Store.h>
#  include <src/raze/vx/hw/x86/memory/StoreNt.h>
#  include <src/raze/vx/hw/x86/memory/MaskStore.h>
#  include <src/raze/vx/hw/x86/merge/Select.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_>
struct stream_ptr;

template <class Options>
struct configurable_store_t : raze::options::conditional_callable<configurable_store_t, Options, aligned_option, nt_option, safe_option> {
    template <any_iterator_or_pointer Mem, simd_type V>
    raze_no_stack_protector raze_always_inline void operator()(_Mem_ it, const V& x) const noexcept {
        return raze::options::__dispatch_call(*this, it, x);
    }

    template <class Mem, simd_type V>
    raze_no_stack_protector raze_always_inline void operator()(stream_ptr<_Mem_> it, const V& x) const noexcept {
        return raze::options::__dispatch_call(*this, it, x);
    }

    template <class Mem, simd_type V>
    static raze_no_stack_protector raze_always_inline auto deferred_call(auto opts, stream_ptr<Mem> it, const V& x) noexcept {
        using Mask = raze::options::fetch_t<raze::options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        return x.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
            auto mem = std::to_address(it);
            _Store_nt<_Abi_::isa>()(mem, ustorage(chunk));
            algorithm::__seek_iter(it, algorithm::bytes_pointer_offset(mem, sizeof(Value) * Chunk::size));
        });
    }

    template <any_iterator_or_pointer Mem, simd_type V>
    static raze_no_stack_protector raze_always_inline auto deferred_call(auto opts, Mem it, const V& x) noexcept {
        using Mask = raze::options::fetch_t<raze::options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        constexpr auto __safe = Options::contains(safe);
		auto mem = std::to_address(it);

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            static_assert(!Options::contains(nt), "The nt option is incompatible with masked load/store.");

            auto __condition = opts[options::condition_key];
            const auto __mask = __condition.mask(options::as<typename _Mask_::condition_type>{});

            if constexpr (Mask::has_alternative)
                return x.__for_each_chunk([] <class Chunk, class MaskChunk, class SourceChunk> (
                    _Chunk& __chunk, const _MaskChunk& __mchunk, const _SourceChunk& __src_chunk, auto& __memory) raze_always_inline_lambda
                {
                    if constexpr (_Options_::contains(aligned)) _Store<_Abi_::isa>()(__memory, _Select<_Abi_::isa, _Value_>()(ustorage(__chunk),
                        ustorage(__src_chunk), ustorage(__mchunk)), __aligned_policy{});
                    else _Store<_Abi_::isa>()(__memory, _Select<_Abi_::isa, _Value_>()(ustorage(__chunk),
                        ustorage(__src_chunk), ustorage(__mchunk)));

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mask.__storage().storage(), __condition.alternative().__storage().storage(), __mem);
            else
                return __x.__for_each_chunk([] <class _Chunk, class _MaskChunk> (
                    _Chunk& __chunk, const _MaskChunk& __mchunk, auto& __memory) raze_always_inline_lambda
                {
                    if constexpr (_Options_::contains(aligned)) _Mask_store<_Abi_::isa, _Value_, __safe>()(__memory, ustorage(__mchunk), ustorage(__chunk), __aligned_policy{});
                    else _Mask_store<_Abi_::isa, _Value_, __safe>()(__memory, ustorage(__mchunk), ustorage(__chunk));

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mask.__storage().storage(), __mem);
        }
        else {
            if constexpr (_Options_::contains(nt)) {
                return __x.__for_each_chunk([] <class _Chunk> (_Chunk& __chunk, auto& __memory) raze_always_inline_lambda {
                    _Store_nt<_Abi_::isa>()(__memory, ustorage(__chunk));
                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mem);
            }
            else {
                return __x.__for_each_chunk([] <class _Chunk> (_Chunk& __chunk, auto& __memory) raze_always_inline_lambda {
                    if constexpr (_Options_::contains(aligned)) _Store<_Abi_::isa>()(__memory, ustorage(__chunk), __aligned_policy{});
                    else _Store<_Abi_::isa>()(__memory, ustorage(__chunk));

                    algorithm::__advance_bytes(__memory, sizeof(_Value_) * _Chunk::size);
                }, __mem);
            }
        }
    }
};

constexpr inline auto __store = raze::options::functor<_Configurable_store>;

__RAZE_VX_NAMESPACE_END
