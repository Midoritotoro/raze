#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::vertical_max", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    rtts::random::generator<T> gen(0x12345678ULL + sizeof(T) * 37 + N);

    alignas(alignof(V)) T arrA[N], arrB[N], arrSrc[N];

    const auto any_domain = rtts::random::domain<T>::standard();

    for (size_t trial = 0; trial < 1000; ++trial) {
        gen.fill(arrA, any_domain);
        gen.fill(arrB, any_domain);
        gen.fill(arrSrc, any_domain);

        V a   = raze::vx::load<V>(arrA);
        V b   = raze::vx::load<V>(arrB);
        V src = raze::vx::load<V>(arrSrc);

        Mask m = rtts::simd::make_random_mask<Mask>();

        auto r1 = raze::vx::vmax(a, b);
        auto r2 = raze::vx::vmax[m](a, b);
        auto r3 = raze::vx::vmax[m, src](a, b);

        RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return raze::math::max(arrA[i], arrB[i]); });
        RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) { return m[i] ? raze::math::max(arrA[i], arrB[i]) : T(0); });
        RTTS_ALL_VALIDATE_VECTOR(r3, [&](auto i) { return m[i] ? raze::math::max(arrA[i], arrB[i]) : arrSrc[i]; });
    }

    gen.fill(arrA, any_domain);

    V a = raze::vx::load<V>(arrA);
    auto r_self_max = raze::vx::vmax(a, a);
    RTTS_ALL_VALIDATE_VECTOR(r_self_max, [&](auto i) { return arrA[i]; });
};

RAZE_TEST_NAMESPACE_END