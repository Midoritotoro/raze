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

/**
 * @ingroup raze_vx_construct
 * @memberof raze::vx
 * @hideinitializer
 * @brief Broadcasts a scalar value to every lane of a SIMD vector.
 *
 * Returns a vector of type `V` where every element is set to the given scalar value `x`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * constexpr inline *unspecified* broadcast;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The scalar value to broadcast into all selected lanes.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A vector of type `V` with `r[i] == x` for every selected lane.
 *
 * ### Example
 * ```cpp
 * using V = raze::vx::simd<int>;
 *
 * auto b1 = raze::vx::broadcast<V>(42);           // [42, 42, ...]
 * auto b2 = raze::vx::broadcast<V>[m](42);        // [42 or 0]
 * auto b3 = raze::vx::broadcast<V>[m, src](42);   // [42 or src[i]]
 * ```
*/
template <simd_type V> 
constexpr inline auto broadcast = options::functor<configurable_broadcast_t<V>::template broadcast>;

__RAZE_VX_NAMESPACE_END
