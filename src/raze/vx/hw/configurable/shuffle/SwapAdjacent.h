#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_swap_adjacent : raze::options::strict_elementwise_callable<_Configurable_swap_adjacent, _Options_> {
    template <simd_type _Type_, sizetype _GroupSize_>
    raze_nodiscard raze_no_stack_protector raze_always_inline _Type_
        operator()(const _Type_& __x, std::integral_constant<sizetype, _GroupSize_> __i) const noexcept
    {
        return raze::options::__dispatch_call(*this, __x, __i);
    }

    template <simd_type _Type_, sizetype _GroupSize_>
    raze_nodiscard static raze_no_stack_protector raze_always_inline auto deferred_call(auto __options,
        const _Type_& __x, std::integral_constant<sizetype, _GroupSize_> __i) noexcept
    {
        return __shuffle(__x, make_swap_adjacent_pattern<_Type_, _GroupSize_>{});
    }

    using callable_tag_type = _Configurable_swap_adjacent;
};

constexpr inline auto __swap_adjacent = raze::options::functor<_Configurable_swap_adjacent>;

__RAZE_VX_NAMESPACE_END
