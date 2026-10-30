#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/construct/Broadcast.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V>
struct configurable_zero_t {
    template <class Options>
    struct zero: options::conditional_callable<zero, Options> {
        raze_nodiscard raze_always_inline V operator()() const noexcept {
            return options::dispatch_call(*this);
        }

        static raze_always_inline auto deferred_call(auto opts) noexcept {
            using Mask = options::fetch_t<options::condition_key, Options>;
            using Value = typename V::value_type;
            using Abi = typename V::abi_type;

            V r;

            auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
                chunk = zero_<Abi::isa, typename Chunk::unwrapped_type, Value>(ustorage<Args>(args)...);
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
};

template <simd_type V> 
constexpr inline auto zero = options::functor<configurable_zero_t<V>::template zero>;

__RAZE_VX_NAMESPACE_END
