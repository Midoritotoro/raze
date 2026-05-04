#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/BitNot.h>
#  include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_not: raze::options::strict_elementwise_callable<_Configurable_not, _Options_> {
    template <simd_or_mask_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_or_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;
        using _Op = std::conditional_t<simd_type<_Type_>, _Not<_Abi_::isa, _Value_>, _Mask_not<_Abi_::isa, _Value_>>;

        _Type_ __result = __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk, const auto& __mask, const auto& __src) raze_always_inline_lambda {
                    __chunk = _Op()(__chunk.data(), __mask.data(), __src.data());
                }, __mask.__storage(), __condition.alternative().__storage());
            else
                __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk, const auto& __mask) raze_always_inline_lambda {
                    __chunk = _Op()(__chunk.data(), __mask.data());
                }, __mask.__storage());
        }
        else {
            __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
                __chunk = _Op()(__chunk.data());
            });
        }

        return __result;
    }

    using callable_tag_type = _Configurable_not;
};

__RAZE_VX_NAMESPACE_END
