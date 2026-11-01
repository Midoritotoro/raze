#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/compare/Less.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_is_less_t: options::conditional_callable<configurable_is_less_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline simd_mask<typename V::value_type, abi_t<V>> 
        operator()(const V& x, const V& y) const noexcept 
    {
        return options::dispatch_call(*this, x, y);
    }

    template <simd_type V>
    raze_nodiscard raze_always_inline simd_mask<typename V::value_type, typename V::abi_type> 
        operator()(const V& x, typename V::value_type y) const noexcept
    {
        return options::dispatch_call(*this, x, V(y));
    }
        
    template <simd_type V>
    raze_nodiscard raze_always_inline simd_mask<typename V::value_type, typename V::abi_type>
        operator()(typename V::value_type x, const V& y) const noexcept
    {
        return options::dispatch_call(*this, V(x), y);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        simd_mask<Value, Abi> r(uninitialized);

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = less_<Abi::isa, Value>(ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];
            r.__for_each_chunk(chunk_op, x.__storage().storage(), y.__storage().storage(), condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, x.__storage().storage(), y.__storage().storage());
        }

        return r;
    }
};

/**
 * @ingroup raze_vx_compare
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the element-wise less-than comparison of two SIMD vectors.
 *
 * Returns a `simd_mask` where each lane is `true` if the corresponding lanes in `x` are less than in `y`,
 * and `false` otherwise.
 *
 * ### Declarations
 * ```cpp
 * // (1) Vector-Vector
 * template <simd_type V>
 * simd_mask<typename V::value_type, typename V::abi_type> operator()(const V& x, const V& y) const noexcept;
 *
 * // (2) Vector-Scalar
 * template <simd_type V>
 * simd_mask<typename V::value_type, typename V::abi_type> operator()(const V& x, typename V::value_type y) const noexcept;
 *
 * // (3) Scalar-Vector
 * template <simd_type V>
 * simd_mask<typename V::value_type, typename V::abi_type> operator()(typename V::value_type x, const V& y) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The target SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The left operand (vector or scalar).
 * - `y` — The right operand (vector or scalar).
 *
 * ### Mask semantics
 * Supports **only** the maskz form (`[m]`). If the mask `m[i]` is `false`, the corresponding lane
 * in the resulting `simd_mask` is `false`.
 *
 * **Merge semantics (`[m, src]`) are not supported** because the result type is a mask, not a vector.
 *
 * ### Return value
 * A `simd_mask` of type `simd_mask<typename V::value_type, typename V::abi_type>` where
 * `r[i] == (x[i] < y[i])` for every selected lane.
*/
constexpr inline auto is_less = options::functor<configurable_is_less_t>;

__RAZE_VX_NAMESPACE_END
