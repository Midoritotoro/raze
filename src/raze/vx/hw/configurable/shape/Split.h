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

constexpr inline auto split = options::functor<configurable_split_t>;

__RAZE_VX_NAMESPACE_END
