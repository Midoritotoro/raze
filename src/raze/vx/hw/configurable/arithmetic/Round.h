#pragma once

#include <src/raze/options/DispatchCall.h>
#include <src/raze/options/Functor.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Round.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_round_t : options::conditional_callable<configurable_round_t, Options> {
    template <floating_point_simd V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <floating_point_simd V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = round_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
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

/**
 * @ingroup raze_vx_arith
 * @memberof raze::vx
 * @hideinitializer
 * @brief Rounds each lane of a floating-point SIMD vector to the nearest integer value, with ties rounded away from zero.
 *
 * ### Declarations
 * ```cpp
 * template <floating_point_simd V>
 * V operator()(const V& x) const noexcept;
 * ```
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
*/
constexpr inline auto round = options::functor<configurable_round_t>;

__RAZE_VX_NAMESPACE_END

