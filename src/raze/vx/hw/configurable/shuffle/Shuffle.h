#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

struct within_chunks_mode {};
constexpr auto within_chunks = raze::options::flag(within_chunks_mode{});
struct within_chunks_option : raze::options::exact_option<within_chunks> {};

template <class _Options_>
struct _Configurable_shuffle: raze::options::strict_elementwise_callable<_Configurable_shuffle, _Options_, within_chunks_option> {
    template <simd_type _Type_, class _Pattern_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x, _Pattern_ __p) const noexcept
        requires(_Type_::size() == _Pattern_::size())
    {
        return raze::options::__dispatch_call(*this, __x, __p);
    }

    template <simd_type _Type_, class _Pattern_>
    raze_nodiscard static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, _Pattern_ __p) noexcept {
        return __shuffle(__x, __p);
    }

    using callable_tag_type = _Configurable_shuffle;
};

__RAZE_VX_NAMESPACE_END
