#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/construct/Broadcast.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V>
struct configurable_all_bits_t {
    template <class Options>
    struct all_bits: options::conditional_callable<all_bits, Options> {
        raze_nodiscard raze_always_inline V operator()() const noexcept {
            return options::dispatch_call(*this);
        }

        static raze_always_inline auto deferred_call(auto opts) noexcept {
            using Mask = options::fetch_t<options::condition_key, Options>;
            using Value = typename V::value_type;
            using Abi = typename V::abi_type;

            V r;

            auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
                chunk = all_bits_<Abi::isa, typename Chunk::unwrapped_type, Value>(ustorage<Args>(args)...);
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
 * @brief Produces a SIMD vector with every bit set to `1`.
 *
 * Returns a vector of type `V` where every bit of every lane is `1`.  
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * constexpr inline *unspecified* all_bits;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A vector of type `V` with `r[i] == ~T(0)` for every selected lane 
 *
 * ### Example
 * ```cpp
 * using V = raze::vx::simd<int>;
 *
 * auto ones = raze::vx::all_bits<V>();           // [~0, ~0, ...]
 * auto z    = raze::vx::all_bits<V>[m]();        // [~0 or 0]
 * auto r    = raze::vx::all_bits<V>[m, src]();   // [~0 or src[i]]
 * ```
*/
template <simd_type V> 
constexpr inline auto all_bits = options::functor<configurable_all_bits_t<V>::template all_bits>;

__RAZE_VX_NAMESPACE_END
