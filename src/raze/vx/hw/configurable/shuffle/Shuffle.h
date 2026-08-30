#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct configurable_shuffle_t: raze::options::conditional_callable<configurable_shuffle_t, Options> {
    template <class Pattern>
    raze_nodiscard raze_always_inline pattern_vector_t<Pattern> operator()(
        const pattern_vector_t<Pattern>& x, Pattern p) const noexcept
    {
        return options::dispatch_call(*this, x, p);
    }

    template <simd_type V, index_simd_type Index>
    raze_nodiscard raze_always_inline V operator()(const V& x, const Index& idx) const noexcept
        requires(index_type_for<Index, V>) 
    {
        return options::dispatch_call(*this, x, idx);
    }

    template <class Pattern>
    static raze_always_inline auto deferred_call(auto opts,
        const pattern_vector_t<Pattern>& x, Pattern p) noexcept
    {
        return shuffle_(x, p);
    }

    template <simd_type V, index_simd_type Index>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const Index& idx) noexcept {
        return shuffle_(x, idx);
    }
};

constexpr inline auto shuffle = options::functor<configurable_shuffle_t>;

__RAZE_VX_NAMESPACE_END
