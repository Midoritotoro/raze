#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V>
using pair_type = std::pair<simd<typename V::value_type, resize_abi_t<abi_t<V>, V::size() / 2>>,
    simd<typename V::value_type, resize_abi_t<abi_t<V>, V::size() - (V::size() / 2)>>>;

template <class Options>
struct configurable_split_t : options::conditional_callable<configurable_split_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline pair_type<V> operator()(const V& x) const noexcept
        requires(V::size() > 1)
    {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline pair_type<V> deferred_call(auto, const V& x) noexcept {
        using First = typename pair_type<V>::first_type;
        using Second = typename pair_type<V>::second_type;
        
        alignas(64) typename V::value_type arr[V::size()];
        vx::store[vx::aligned](arr, x);

        return { vx::load<First>[vx::aligned](arr), vx::load<Second>[vx::aligned](arr + First::size()) };
    }
};

/**
 * @ingroup raze_vx_shape
 * @memberof raze::vx
 * @hideinitializer
 * @brief Splits a SIMD vector into two smaller SIMD vectors.
 *
 * Divides the input vector `x` into two halves and returns them as a `std::pair`.
 * If `V::size()` is odd, the first vector contains exactly `V::size() / 2` elements,
 * and the second vector contains the remaining `V::size() - (V::size() / 2)` elements.
 *
 * ### Declarations
 * ```cpp
 * template <simd_type V>
 * pair_type<V> operator()(const V& x) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The source SIMD type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input vector to split.
 *
 * ### Constraints
 * - `V::size() > 1` — The input vector must contain at least two elements.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * A `std::pair` containing two SIMD vectors. The sum of their lane counts equals the lane count of `V`.
*/
constexpr inline auto split = options::functor<configurable_split_t>;

__RAZE_VX_NAMESPACE_END
