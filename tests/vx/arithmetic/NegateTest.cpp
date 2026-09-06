#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::neg", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrA[N], arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrSrc[i] = T(100 + i);
    }

    V a = raze::vx::load<V>(arrA);
    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    auto r1 = raze::vx::neg(a);
    auto r2 = raze::vx::neg[m](a);
    auto r3 = raze::vx::neg[m, src](a);

    RTTS_ALL_VALIDATE_BITS(r1, [&](auto i) { return T(-arrA[i]); });
    RTTS_ALL_VALIDATE_BITS(r2, [&](auto i) { return m[i] ? T(-arrA[i]) : T(0); });
    RTTS_ALL_VALIDATE_BITS(r3, [&](auto i) { return m[i] ? T(-arrA[i]) : arrSrc[i]; });
};