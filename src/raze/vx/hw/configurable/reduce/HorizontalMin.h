#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/HorizontalMin.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_hmin : raze::options::strict_elementwise_callable<_Configurable_hmin, _Options_> {
    template <simd_type _Type_>
    raze_nodiscard raze_always_inline typename _Type_::value_type operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        _Value_ __result = 0;
        
        auto __chunk_op = [&] <class ... _Args_> (_Args_&& ... __args) raze_always_inline_lambda {
            __result = std::min(_Horizontal_min<_Abi_::isa, _Value_>()(__storage_unwrap<_Args_>(__args)...), __result);
        };

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative) {
                auto __alternative = __condition.alternative();
                __result = __mask[0] ? __x[0] : __alternative[0];
                __x.__for_each_chunk(__chunk_op, __mask.__storage().storage(), __alternative.__storage().storage());
            }
            else {
                __result = __mask[0] ? __x[0] : 0;
                __x.__for_each_chunk(__chunk_op, __mask.__storage().storage());
            }
        }
        else {
            __result = __x[0];
            __x.__for_each_chunk(__chunk_op);
        }

        return __result;
    }

    using callable_tag_type = _Configurable_hmin;
};

__RAZE_VX_NAMESPACE_END
