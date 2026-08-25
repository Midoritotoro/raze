#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Storage.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Abs.h>
#endif // defined(raze_processor_x86)


__RAZE_MATH_NAMESPACE_BEGIN

template <class Options>
struct configurable_abs_t: raze::options::conditional_callable<configurable_abs_t, Options> {
    template <vx::arithmetic_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <vx::simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <vx::arithmetic_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, _Options_>;

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) return mask ? (x < 0 ? -x : x) : condition.alternative();
            else return mask ? (x < 0 ? -x : x) : 0;
        }
        else return x < 0 ? -x : x;
    }

    template <vx::simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, _Options_>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = vx::abs_<Abi::isa, Value>(vx::ustorage(chunk), vx::ustorage<Args>(args)...);
        };

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, mask.__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, mask.__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

__RAZE_MATH_NAMESPACE_END

