#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shape/Concat.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class ... Ts>
using concatenated_simd = simd<typename T::value_type, resize_abi_t<typename T::abi_type,
    abi_t<T>::size + (abi_t<Ts>::size + ...)>>;

template <class ... Ts> 
concept effective_concatenation_available = same_abi_isa<Ts...> && !has_any_scalar_chunks<Ts...>;

template <class Options>
struct configurable_concat_t: options::conditional_callable<configurable_concat_t, Options> {
    template <simd_type T, class ... Ts>
    raze_nodiscard raze_always_inline concatenated_simd<T, Ts...> operator()(
        const T& x, const Ts& ... xs) const noexcept
            requires(sizeof...(Ts) > 0 && same_abi_isa<T, Ts...>)
    {
        return options::dispatch_call(*this, x, xs...);
    }

    template <simd_type T, class ... Ts>
    static raze_always_inline auto deferred_call(auto opts, const T& x, const Ts& ... xs) noexcept {
        using Concatenated = concatenated_simd<T, Ts...>;
        using Value = typename Concatenated::value_type;
        using Abi = typename Concatenated::abi_type;

        Concatenated r;
        
        alignas(64) Value arr[Abi::size];
        Value* ptr = arr;

        ([&]() raze_always_inline_lambda { store[aligned](ptr, xs); ptr += xs.size(); }(), ...);
        r = load<Concatenated>[aligned](arr);

        return r;
    }
};

constexpr inline auto concat = options::functor<configurable_concat_t>;

__RAZE_VX_NAMESPACE_END
