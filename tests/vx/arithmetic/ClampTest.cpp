#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::clamp", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrV[N], arrLow[N], arrHigh[N], arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrV[i] = T(i);
        arrLow[i] = T(N / 4);
        arrHigh[i] = T(N - N / 4 - 1);
        arrSrc[i] = T(100 + i);
    }

    V v = raze::vx::load<V>(arrV);
    V low = raze::vx::load<V>(arrLow);
    V high = raze::vx::load<V>(arrHigh);
    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    auto scalar_eval = [&](size_t i) {
        return raze::math::min(raze::math::max(arrV[i], arrLow[i]), arrHigh[i]);
    };

    auto r1 = raze::vx::clamp(v, low, high);
    auto r2 = raze::vx::clamp[m](v, low, high);
    auto r3 = raze::vx::clamp[m, src](v, low, high);

    RTTS_ALL_VALIDATE_BITS(r1, [&](auto i) { return scalar_eval(i); });
    RTTS_ALL_VALIDATE_BITS(r2, [&](auto i) { return m[i] ? scalar_eval(i) : T(0); });
    RTTS_ALL_VALIDATE_BITS(r3, [&](auto i) { return m[i] ? scalar_eval(i) : arrSrc[i]; });
};