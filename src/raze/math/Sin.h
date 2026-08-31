#pragma once 

#include <raze/vx/Simd.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::floating_point T>
raze_always_inline T fast_sin_impl(T x) noexcept {
    constexpr auto sine_table_size = 256;

    auto si = int(x * (T(0.5) * sine_table_size / pi));
    auto ci = int(si + sine_table_size / 4);

    const auto d = T(x - si * (T(2.0) * pi / sine_table_size));

    si &= (sine_table_size - 1);
    ci &= (sine_table_size - 1);

    return sine_table[si] + (sine_table[ci] - T(0.5) * sine_table[si] * d) * d;
}

template <std::floating_point T>
raze_always_inline T sin_impl(T x) noexcept {
    return std::sin(x);
}

template <vx::floating_point_simd V>
raze_always_inline V sin_impl(const V& x) noexcept {
    V r;

    for (auto i = 0; i < x.size(); ++i)
        r[i] = sin_impl(x[i]);
    
    return r;
}

template <class Options>
struct configurable_sin_t: options::conditional_callable<configurable_sin_t, Options> {
    template <vx::floating_point_simd_or_scalar_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <std::floating_point T>
    static raze_always_inline auto deferred_call(auto opts, const T& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) return mask ? sin_impl(x) : condition.alternative();
            else return mask ? sin_impl(x) : 0;
        }
        else return sin_impl(x);
    }

    template <vx::floating_point_simd V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = options[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) 
                return vx::select[mask, condition.alternative()](sin_impl(x));
            else return vx::select[mask](sin_impl(x));
        }
        else return sin_impl(x);
    }
};

constexpr inline auto sin = options::functor<configurable_sin_t>;

__RAZE_MATH_NAMESPACE_END
