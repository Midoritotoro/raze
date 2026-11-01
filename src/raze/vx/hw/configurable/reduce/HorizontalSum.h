#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/ReduceAdd.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_hsum_t: options::conditional_callable<configurable_hsum_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline reduce_type<typename V::value_type> operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        reduce_type<typename V::value_type> r = 0;
        
        auto chunk_op = [&] <class ... Args> (Args&& ... args) raze_always_inline_lambda {
            r += reduce_add_<Abi::isa, Value>(ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        else {
            x.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

/**
 * @ingroup raze_vx_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the horizontal sum of a SIMD vector.
 *
 * Returns the sum of all lanes of the input vector `x`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * reduce_type<typename V::value_type> operator()(const V& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The source SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input vector to reduce.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics :
 * - **Maskz (`[m]`)**: Unselected lanes are evaluated as `0` during the reduction.
 * - **Merge (`[m, src]`)**: Unselected lanes are evaluated as `src[i]` during the reduction.
 *
 * ### Return value
 * The sum of type `reduce_type<typename V::value_type>` across all (selected) lanes.
*/
constexpr inline auto horizontal_sum = options::functor<configurable_hsum_t>;

/**
 * @ingroup raze_vx_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Alias for @ref horizontal_sum.
*/
constexpr inline auto hsum = horizontal_sum;

__RAZE_VX_NAMESPACE_END
