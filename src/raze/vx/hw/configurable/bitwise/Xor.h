#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/BitXor.h>
#  include <src/raze/vx/hw/x86/mask/operations/MaskXor.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_xor_t: options::conditional_callable<configurable_xor_t, Options> {
    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x, const T& y) const noexcept {
        return options::dispatch_call(*this, x, y);
    }

    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x, typename T::value_type y) const noexcept {
        return options::dispatch_call(*this, x, T(y));
    }
        
    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(typename T::value_type x, const T& y) const noexcept {
        return options::dispatch_call(*this, T(x), y);
    }

    template <simd_or_mask_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x, const T& y) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename T::value_type;
        using Abi = typename T::abi_type;

        T r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            if constexpr (simd_mask_type<T>) chunk = mask_xor_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
            else chunk = bit_xor_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

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
 * @ingroup raze_vx_bitwise
 * @memberof raze::vx
 * @hideinitializer
 * @brief Computes the element-wise bitwise XOR of two SIMD vectors or masks.
 *
 * Returns a value of type `T` where each lane is the bitwise XOR of the corresponding lanes in `x` and `y`.
 * This operation is overloaded for both standard SIMD vectors (`simd`) and masks (`simd_mask`).
 *
 * ### Declarations
 * ```cpp
 * // (1) Type-Type
 * template <simd_or_mask_type T>
 * T operator()(const T& x, const T& y) const noexcept;
 *
 * // (2) Type-Scalar
 * template <simd_or_mask_type T>
 * T operator()(const T& x, typename T::value_type y) const noexcept;
 *
 * // (3) Scalar-Type
 * template <simd_or_mask_type T>
 * T operator()(typename T::value_type x, const T& y) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `T` — The target SIMD vector or mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The left operand (vector/mask or scalar).
 * - `y` — The right operand (vector/mask or scalar).
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 *
 * ### Return value
 * A value of type `T` with `r[i] == x[i] ^ y[i]` for every selected lane.
*/
constexpr inline auto bit_xor = options::functor<configurable_xor_t>;

__RAZE_VX_NAMESPACE_END
