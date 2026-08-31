#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/LeftShift.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_shl_t: options::conditional_callable<configurable_shl_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x, u32 shift) const noexcept {
        return options::dispatch_call(*this, x, shift);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, u32 shift) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Abi = typename V::abi_type;
        using Value = typename V::value_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&&... args) raze_always_inline_lambda {
            chunk = left_shift_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        V r = x;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, shift, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, shift, condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, shift);
        }

        return r;
    }
};

constexpr inline auto bit_shl = options::functor<configurable_shl_t>;

__RAZE_VX_NAMESPACE_END
