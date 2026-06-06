#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Splat.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _Index_>
struct _Configurable_splat {
    template <class _Options_>
    struct __impl: raze::options::strict_elementwise_callable<__impl, _Options_> {
        template <simd_type _Type_>
        raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x) const noexcept {
            return raze::options::__dispatch_call(*this, __x);
        }

        template <simd_type _Type_>
        raze_nodiscard static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
            return __splat(__x, make_splat_pattern<_Type_, _Index_>{});
        }

        using callable_tag_type = __impl;
    };
};

template <sizetype _Index_> constexpr inline auto __splat_impl = raze::options::functor<typename _Configurable_splat<_Index_>::__impl>;


__RAZE_VX_NAMESPACE_END
