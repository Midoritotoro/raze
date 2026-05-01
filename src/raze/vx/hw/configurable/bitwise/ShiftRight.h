#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/ShiftRight.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_shr: raze::options::strict_elementwise_callable<_Configurable_shr, _Options_> {
    template <simd_type _Simd_>
    raze_nodiscard raze_always_inline _Simd_ operator()(const _Simd_& __x, uint32 __shift) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __shift);
    }

    template <simd_type _Simd_>
    static raze_always_inline auto deferred_call(auto __options, const _Simd_& __x, uint32 __shift) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

        auto __chunk_op = [&] <class _Chunk, class ... _Args> (_Chunk& __chunk, _Args&&... __args) raze_always_inline_lambda {
            __chunk = _Shift_right<_Simd_::__isa, typename _Simd_::value_type>()(__chunk, std::forward<_Args>(__args)...);
        };

        _Simd_ __result = __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk(__chunk_op, __shift, __data(__mask), __data(__condition.alternative()));
            else
                __result.__for_each_chunk(__chunk_op, __shift, __data(__mask));
        }
        else {
            __result.__for_each_chunk(__chunk_op);
        }

        return __result;
    }

    using callable_tag_type = _Configurable_shr;
};

__RAZE_VX_NAMESPACE_END
