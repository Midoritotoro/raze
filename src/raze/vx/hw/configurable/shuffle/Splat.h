#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Splat.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype Index>
struct configurable_splat_t {
    template <class Options>
    struct impl: options::conditional_callable<impl, Options> {
        template <simd_type V>
        raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
            return options::dispatch_call(*this, x);
        }

        template <simd_type V>
        raze_nodiscard static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
            return splat_(x, make_splat_pattern<V, Index>{});
        }
    };
};

template <sizetype Index> constexpr inline auto splat = options::functor<configurable_splat_t<Index>::template impl>;

__RAZE_VX_NAMESPACE_END
