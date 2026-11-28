#pragma once

#include <src/raze/options/DispatchCall.h>
#include <src/raze/options/Functor.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Rem.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_rem_t : raze::options::conditional_callable<configurable_rem_t, Options> {
    template <integral_simd V>
    raze_nodiscard raze_always_inline V operator()(const V& x, const V& y) const noexcept {
        return options::dispatch_call(*this, x, y);
    }

    template <integral_simd V>
    raze_nodiscard raze_always_inline V operator()(const V& x, typename V::value_type y) const noexcept {
        return options::dispatch_call(*this, x, y);
    }

    template <integral_simd V>
    raze_nodiscard raze_always_inline V operator()(typename V::value_type x, const V& y) const noexcept {
        return options::dispatch_call(*this, V(x), y);
    }

    template <integral_simd V, std::integral Divisor>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const Divisor& y) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&&... args) raze_always_inline_lambda {
            chunk = rem_<Abi::isa, Value>(ustorage(chunk), ustorage(std::forward<Args>(args))...);
        };

        V r = x;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, static_cast<Value>(y), condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, static_cast<Value>(y), condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, static_cast<Value>(y));
        }

        return r;
    }

    template <integral_simd V, integral_simd Divisor>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const Divisor& y) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&&... args) raze_always_inline_lambda {
            chunk = rem_<Abi::isa, Value>(ustorage(chunk), ustorage(std::forward<Args>(args))...);
        };

        V r = x;

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

/**
 * @ingroup raze_vx_arith
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the element-wise remainder of integer division of two SIMD vectors.
 *
 * Returns a vector of type `V` where each lane is the remainder `x[i] % y[i]` of the corresponding lanes.
 *
 * ### Declarations
 * ```cpp
 * // (1) Vector-Vector
 * template <integral_simd V>
 * V operator()(const V& x, const V& y) const noexcept;
 *
 * // (2) Vector-Scalar
 * template <integral_simd V>
 * V operator()(const V& x, typename V::value_type y) const noexcept;
 *
 * // (3) Scalar-Vector
 * template <integral_simd V>
 * V operator()(typename V::value_type x, const V& y) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target integral SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The dividend (vector or scalar).
 * - `y` — The divisor (vector or scalar).
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A vector of type `V` with `r[i] == x[i] % y[i]` for every selected lane.
*/
constexpr inline auto rem = options::functor<configurable_rem_t>;

__RAZE_VX_NAMESPACE_END

