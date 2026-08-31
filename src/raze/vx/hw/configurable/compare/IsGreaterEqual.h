#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/compare/GreaterEqual.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_is_greater_equal_t: options::conditional_callable<configurable_is_greater_equal_t, Options> {
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

        simd_mask<Value, Abi> r;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = greater_equal_<Abi::isa, Value>(ustorage<Args>(args)...);
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

constexpr inline auto is_greater_equal = options::functor<configurable_is_greater_equal_t>;

__RAZE_VX_NAMESPACE_END
