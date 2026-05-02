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

        auto __chunk_op = [&] <class _Chunk, class ... _Args> (_Chunk & __chunk, _Args&&... __args) raze_always_inline_lambda {
            using _Op = std::conditional_t<simd_type<_Type_>, _Not<_Type_::__isa, _Value_>, _Mask_not<_Type_::__isa, _Value_>>;
            __chunk = _Op()(__chunk, std::forward<_Args>(__args)...);
        };

        _Type_ __result = __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk(__chunk_op, __data(__mask), __data(__condition.alternative()));
            else
                __result.__for_each_chunk(__chunk_op, __data(__mask));
        }
        else {
            __result.__for_each_chunk(__chunk_op);
        }

        return __result;
    }

    using callable_tag_type = _Configurable_not;
};

__RAZE_VX_NAMESPACE_END
