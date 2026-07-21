#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/math/MathConstants.h>
#include <src/raze/math/SinTables.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <std::floating_point _Type_>
raze_always_inline _Type_ __fast_cos(const _Type_& __x) noexcept {
    constexpr auto __sine_table_size = 256;

    const auto __ci = int(__x * (_Type_(0.5) * __sine_table_size / pi)) & (__sine_table_size - 1);
    const auto __si = int(__ci + __sine_table_size / 4) & (__sine_table_size - 1);
    const auto __d = __x - __ci * (_Type_(2.0) * pi / __sine_table_size);

    return __sine_table[__si] - (__sine_table[__ci] + _Type_(0.5) * __sine_table[__si] * __d) * __d;
}

template <std::floating_point _Type_>
raze_always_inline _Type_ __cos(const _Type_& __x) noexcept {
    return std::cos(__x);
}

template <vx::simd_type _Simd_>
raze_always_inline _Simd_ __cos(const _Simd_& __x) noexcept
    requires(std::floating_point<typename _Simd_::value_type>)
{
    _Simd_ __r;

    for (auto __i = 0; __i < __x.size(); ++__i)
        __r[__i] = __cos(__x[__i]);

    return __r;
}

template <class _Options_>
struct _Configurable_cos: raze::options::strict_elementwise_callable<_Configurable_cos, _Options_> {
    template <vx::floating_point_simd_or_scalar_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <std::floating_point _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative) return __mask ? __cos(__x) : __condition.alternative();
            else return __mask ? __cos(__x) : 0;
        }
        else return __cos(__x);
    }

    template <vx::floating_point_simd _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative) 
                return vx::__select[__mask.__storage().storage(), __condition.alternative().__storage().storage()](__cos(__x));
            else return vx::__select[__mask.__storage().storage()](__cos(__x));
        }
        else return __cos(__x);
    }

    using callable_tag_type = _Configurable_cos;
};

__RAZE_MATH_NAMESPACE_END
