#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_reverse: raze::options::strict_elementwise_callable<_Configurable_reverse, _Options_> {
    //template <simd_type _Type_, index_simd_type _Index_>
    //raze_nodiscard raze_always_inline _Type_ operator()(
    //    const _Type_& __x, const _Index_& __index) const noexcept
    //        requires(_Type_::size() == _Index_::size())
    //{
    //    return raze::options::__dispatch_call(*this, __x, __index);
    //}

    template <simd_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    //template <simd_type _Type_, index_simd_type _Index_>
    //static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, const _Index_& __index) noexcept {
    //    
    //}

    template <simd_type _Type_>
    raze_nodiscard static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        return __reverse(__x);
    }

    using callable_tag_type = _Configurable_reverse;
};

__RAZE_VX_NAMESPACE_END
