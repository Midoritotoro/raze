#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_neg_t: options::conditional_callable<configurable_neg_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = negate_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

constexpr inline auto neg = options::functor<configurable_neg_t>;

__RAZE_VX_NAMESPACE_END
