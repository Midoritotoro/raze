#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/construct/Broadcast.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V>
struct configurable_broadcast_t {
    template <class Options>
    struct broadcast : options::conditional_callable<broadcast, Options> {
        template <arithmetic_type T>
        raze_nodiscard raze_always_inline V operator()(T x) const noexcept {
            return options::dispatch_call(*this, x);
        }

        template <arithmetic_type T>
        static raze_always_inline auto deferred_call(auto opts, T x) noexcept {
            using Mask = options::fetch_t<options::condition_key, Options>;
            using Abi = typename V::abi_type;

            V r;

            auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
                chunk = broadcast_<Abi::isa, typename Chunk::unwrapped_type>(x, ustorage<Args>(args)...);
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
constexpr inline auto broadcast = options::functor<configurable_broadcast_t<V>::template broadcast>;

__RAZE_VX_NAMESPACE_END
