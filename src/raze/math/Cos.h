#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::floating_point T>
raze_always_inline T fast_cos_impl(T x) noexcept {
    constexpr auto sine_table_size = 256;

    const auto ci = int(x * (T(0.5) * sine_table_size / pi)) & (sine_table_size - 1);
    const auto si = int(ci + sine_table_size / 4) & (sine_table_size - 1);
    const auto d = x - ci * (T(2.0) * pi / sine_table_size);

    return sine_table[si] - (sine_table[ci] + T(0.5) * sine_table[si] * d) * d;
}

template <std::floating_point T>
raze_always_inline T cos_impl(const T& x) noexcept {
    return std::cos(x);
}

template <vx::floating_point_simd V>
raze_always_inline V cos_impl(const V& x) noexcept {
    V r;

    for (auto i = 0; i < x.size(); ++i)
        r[i] = cos_impl(x[i]);

    return r;
}

template <class Options>
struct configurable_cos_t: raze::options::conditional_callable<configurable_cos_t, Options> {
    template <vx::floating_point_simd_or_scalar_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <std::floating_point T>
    static raze_always_inline auto deferred_call(auto opts, T x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) return mask ? cos_impl(x) : condition.alternative();
            else return mask ? cos_impl(x) : 0;
        }
        else return cos_impl(x);
    }

    template <vx::floating_point_simd V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative) 
                return vx::select[mask, condition.alternative()](cos_impl(x));
            else return vx::select[mask](cos_impl(x));
        }
        else return cos_impl(x);
    }
};

__RAZE_MATH_NAMESPACE_END
