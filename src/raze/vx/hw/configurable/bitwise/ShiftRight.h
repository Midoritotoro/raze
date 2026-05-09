#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/RightShift.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_shr: raze::options::strict_elementwise_callable<_Configurable_shr, _Options_> {
    template <simd_type _Simd_>
    raze_nodiscard raze_always_inline _Simd_ operator()(const _Simd_& __x, u32 __shift) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __shift);
    }

    template <simd_type _Simd_>
    static raze_always_inline auto deferred_call(auto __options, const _Simd_& __x, u32 __shift) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Abi_ = typename _Simd_::abi_type;
        using _Value_ = typename _Simd_::value_type;

        auto __chunk_op = [&] <class _Chunk, class ... _Args> (_Chunk& __chunk, _Args&&... __args) raze_always_inline_lambda {
            __chunk = _Right_shift<_Abi_::isa, _Value_>()(__storage_unwrap(__chunk), __storage_unwrap<_Args>(__args)...);
        };

        _Simd_ __result = __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk(__chunk_op, __shift, __mask.__storage().storage(), __condition.alternative().__storage().storage());
            else
                __result.__for_each_chunk(__chunk_op, __shift, __mask.__storage().storage());
        }
        else {
            __result.__for_each_chunk(__chunk_op, __shift);
        }

        return __result;
    }

    using callable_tag_type = _Configurable_shr;
};

__RAZE_VX_NAMESPACE_END
