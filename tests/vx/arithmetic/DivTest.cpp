#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::div", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrA[N], arrB[N], arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T(N - i);
        arrSrc[i] = T(100 + i);
    }

    V a = raze::vx::load<V>(arrA);
    V b = raze::vx::load<V>(arrB);
    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    auto r1 = raze::vx::div(a, b);
    auto r2 = raze::vx::div[m](a, b);
    auto r3 = raze::vx::div[m, src](a, b);
    auto r4 = raze::vx::div(a, b[0]);
    auto r5 = raze::vx::div[m](a, b[0]);
    auto r6 = raze::vx::div[m, src](a, b[0]);


    RTTS_ALL_VALIDATE_BITS(r1, [&](auto i) { return T(arrA[i] / arrB[i]); });
    RTTS_ALL_VALIDATE_BITS(r2, [&](auto i) { return m[i] ? T(arrA[i] / arrB[i]) : T(0); });
    RTTS_ALL_VALIDATE_BITS(r3, [&](auto i) { return m[i] ? T(arrA[i] / arrB[i]) : arrSrc[i]; });
    RTTS_ALL_VALIDATE_BITS(r4, [&](auto i) { return T(arrA[i] / arrB[0]); });
    RTTS_ALL_VALIDATE_BITS(r5, [&](auto i) { return m[i] ? T(arrA[i] / arrB[0]) : T(0); });
    RTTS_ALL_VALIDATE_BITS(r6, [&](auto i) { return m[i] ? T(arrA[i] / arrB[0]) : arrSrc[i]; });
};