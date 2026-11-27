#pragma once 

#include <src/raze/options/DispatchCall.h>
#include <src/raze/options/Functor.h>
#include <src/raze/vx/Storage.h>
#include <cmath>

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
    static raze_always_inline T scalar_abs(const T& x) noexcept {
        if constexpr (std::is_unsigned_v<T>)
            return x;
        else if constexpr (std::floating_point<T>)
            return std::fabs(x);
        else {
            using U = std::make_unsigned_t<T>;
            return x < 0 ? static_cast<T>(U(0) - static_cast<U>(x)) : x;
        }
    }

    template <vx::arithmetic_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) return mask ? scalar_abs(x) : condition.alternative();
            else return mask ? scalar_abs(x) : T(0);
        }
        else return scalar_abs(x);
    }

    template <vx::simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = vx::abs_<Abi::isa, Value>(vx::ustorage(chunk), vx::ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
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

constexpr inline auto abs = options::functor<configurable_abs_t>;

__RAZE_MATH_NAMESPACE_END

