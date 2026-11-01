#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/HorizontalMin.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_hmin_t: options::conditional_callable<configurable_hmin_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline typename V::value_type operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        Value r = 0;
        
        auto chunk_op = [&] <class ... Args> (Args&& ... args) raze_always_inline_lambda {
            r = std::min(horizontal_min_<Abi::isa, Value>(ustorage<Args>(args)...), r);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative) {
                auto alternative = condition.alternative();
                r = condition.mask()[std::integral_constant<sizetype, 0>{}] ? x[std::integral_constant<sizetype, 0>{}] : alternative[std::integral_constant<sizetype, 0>{}];
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage(), alternative.__storage().storage());
            }
            else {
                r = condition.mask()[std::integral_constant<sizetype, 0>{}] ? x[std::integral_constant<sizetype, 0>{}] : 0;
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
            }
        }
        else {
            r = x[std::integral_constant<sizetype, 0>{}];
            x.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

/**
 * @ingroup raze_vx_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the horizontal minimum of a SIMD vector.
 *
 * Returns the minimum value among all lanes of the input vector `x`.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * typename V::value_type operator()(const V& x) const noexcept;
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
 * The minimum element of type `V::value_type` across all (selected) lanes.
*/
constexpr inline auto horizontal_min = options::functor<configurable_hmin_t>;

/**
 * @ingroup raze_vx_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Alias for @ref horizontal_min.
*/
constexpr inline auto hmin = horizontal_min;

__RAZE_VX_NAMESPACE_END
