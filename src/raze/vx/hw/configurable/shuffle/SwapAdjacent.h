#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_swap_adjacent_t : options::conditional_callable<configurable_swap_adjacent_t, Options> {
    template <simd_type V, sizetype GroupSize>
    raze_nodiscard raze_always_inline V operator()(const V& x, std::integral_constant<sizetype, GroupSize> i) const noexcept {
        return options::dispatch_call(*this, x, i);
    }

    template <simd_type V, sizetype GroupSize>
    static raze_always_inline auto deferred_call(auto opts,
        const V& x, std::integral_constant<sizetype, GroupSize> i) noexcept
    {
        return shuffle_(x, make_swap_adjacent_pattern<V, GroupSize>{});
    }
};

constexpr inline auto swap_adjacent = options::functor<configurable_swap_adjacent_t>;

__RAZE_VX_NAMESPACE_END
