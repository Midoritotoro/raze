#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_mul_t: options::conditional_callable<configurable_mul_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, const V& y) const noexcept {
        return options::dispatch_call(*this, x, y);
    }

    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, typename V::value_type y) const noexcept {
        return options::dispatch_call(*this, x, V(y));
    }
        
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(typename V::value_type x, const V& y) const noexcept {
        return options::dispatch_call(*this, V(x), y);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y) noexcept {
        using Mask = options::fetch_toptions::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = mul_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, y.__storage().storage(), condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, y.__storage().storage(), condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, y.__storage().storage());
        }

        return r;
    }
};

constexpr inline auto mul = options::functor<configurable_mul_t>;

__RAZE_VX_NAMESPACE_END
