#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::div", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    rtts::random::generator<T> gen(0x12345678ULL + sizeof(T) * 37 + N);

    alignas(alignof(V)) T arrA[N], arrB[N], arrSrc[N];

    const auto any_domain = rtts::random::domain<T>::standard();
    const auto non_zero   = rtts::random::domain<T>::non_zero();

    for (size_t trial = 0; trial < 1000; ++trial) {
        gen.fill(arrA, any_domain);
        gen.fill(arrB, non_zero);     
        gen.fill(arrSrc, any_domain);

        V a   = raze::vx::load<V>(arrA);
        V b   = raze::vx::load<V>(arrB);
        V src = raze::vx::load<V>(arrSrc);

        Mask m = rtts::simd::make_random_mask<Mask>();

        auto r1 = raze::vx::div(a, b);
        auto r2 = raze::vx::div[m](a, b);
        auto r3 = raze::vx::div[m, src](a, b);
        auto r4 = raze::vx::div(a, b[0]);
        auto r5 = raze::vx::div[m](a, b[0]);
        auto r6 = raze::vx::div[m, src](a, b[0]);

        RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return T(arrA[i] / arrB[i]); });
        RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) { return m[i] ? T(arrA[i] / arrB[i]) : T(0); });
        RTTS_ALL_VALIDATE_VECTOR(r3, [&](auto i) { return m[i] ? T(arrA[i] / arrB[i]) : arrSrc[i]; });
        RTTS_ALL_VALIDATE_VECTOR(r4, [&](auto i) { return T(arrA[i] / arrB[0]); });
        RTTS_ALL_VALIDATE_VECTOR(r5, [&](auto i) { return m[i] ? T(arrA[i] / arrB[0]) : T(0); });
        RTTS_ALL_VALIDATE_VECTOR(r6, [&](auto i) { return m[i] ? T(arrA[i] / arrB[0]) : arrSrc[i]; });
    }

    gen.fill(arrA, any_domain);
    for (size_t i = 0; i < N; ++i) arrB[i] = T(1);
    
    V a = raze::vx::load<V>(arrA);
    V ones = raze::vx::load<V>(arrB);
    auto r_one = raze::vx::div(a, ones);
    RTTS_ALL_VALIDATE_VECTOR(r_one, [&](auto i) { return arrA[i]; });
};

RAZE_TEST_NAMESPACE_END