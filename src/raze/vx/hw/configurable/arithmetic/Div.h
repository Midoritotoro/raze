#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Div.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_div : raze::options::strict_elementwise_callable<_Configurable_div, _Options_> {
    template <simd_type _Simd_>
    raze_nodiscard raze_always_inline _Simd_ operator()(const _Simd_& __x, const _Simd_& __y) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __y);
    }

    template <simd_type _Simd_>
    raze_nodiscard raze_always_inline _Simd_ operator()(const _Simd_& __x, typename _Simd_::value_type __y) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __y);
    }

    template <simd_type _Simd_>
    raze_nodiscard raze_always_inline _Simd_ operator()(typename _Simd_::value_type __x, const _Simd_& __y) const noexcept {
        return raze::options::__dispatch_call(*this, _Simd_(__x), __y);
    }

    template <simd_type _Simd_, simd_or_arithmetic_type _Divisor_>
    static raze_always_inline auto deferred_call(auto __options, const _Simd_& __x, const _Divisor_& __y) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Simd_::value_type;
        using _Abi_ = typename _Simd_::abi_type;

        auto __chunk_op = [&] <class _Chunk, class ... _Args> (_Chunk& __chunk, _Args&&... __args) raze_always_inline_lambda {
            __chunk = _Div<_Abi_::isa, _Value_>()(__storage_unwrap(__chunk), __storage_unwrap(std::forward<_Args>(__args))...);
        };

        _Simd_ __result = __x;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk(__chunk_op, __y, __data(__mask), __data(__condition.alternative()));
            else
                __result.__for_each_chunk(__chunk_op, __data(__y), __data(__mask));
        }
        else {
            __result.__for_each_chunk(__chunk_op, __data(__y));
        }

        return __result;
    }

    using callable_tag_type = _Configurable_div;
};

__RAZE_VX_NAMESPACE_END
