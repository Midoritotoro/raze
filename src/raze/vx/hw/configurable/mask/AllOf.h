#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AllOf.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_all_of: raze::options::strict_elementwise_callable<_Configurable_all_of, _Options_> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline bool operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                return __x.__for_each_chunk_all_of([&] <class _Chunk> (const _Chunk& __chunk, const auto& __mask, const auto& __src) raze_always_inline_lambda {
                    return _All_of<_Abi_::isa, _Chunk::size, _Value_>()(__chunk.data(), __mask.data(), __src.data());
                }, __mask.__storage(), __condition.alternative().__storage());
            else
                return __x.__for_each_chunk_all_of([&] <class _Chunk> (const _Chunk& __chunk, const auto& __mask) raze_always_inline_lambda {
                    return _All_of<_Abi_::isa, _Chunk::size, _Value_>()(__chunk.data(), __mask.data());
                }, __mask.__storage());
        }
        else {
            return __x.__for_each_chunk_all_of([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
                return _All_of<_Abi_::isa, _Chunk::size, _Value_>()(__chunk.data());
            });
        }
    }

    using callable_tag_type = _Configurable_all_of;
};

__RAZE_VX_NAMESPACE_END
