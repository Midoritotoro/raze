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

/**
 * @ingroup raze_vx_construct
 * @memberof raze::vx
 * @hideinitializer
 * @brief Produces a SIMD vector with every bit set to `0`.
 *
 * Returns a vector of type `V` where every lane is initialized to `0`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * constexpr inline *unspecified* zero;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics :
 *
 * ### Return value
 * A vector of type `V` with `r[i] == 0` for every selected lane.
 *
 * ### Example
 * ```cpp
 * using V = raze::vx::simd<int>;
 *
 * auto z1 = raze::vx::zero<V>();           // [0, 0, ...]
 * auto z2 = raze::vx::zero<V>[m]();        // [0, 0, ...] (same as unmasked)
 * auto z3 = raze::vx::zero<V>[m, src]();   // [0 or src[i]]
 * ```
*/
template <simd_type V> 
constexpr inline auto zero = options::functor<configurable_zero_t<V>::template zero>;

__RAZE_VX_NAMESPACE_END
