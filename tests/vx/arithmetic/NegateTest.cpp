#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::neg", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    rtts::random::generator<T> gen(0x12345678ULL + sizeof(T) * 37 + N);

    alignas(alignof(V)) T arrA[N], arrSrc[N];

    const auto any_domain = rtts::random::domain<T>::standard();

    for (size_t trial = 0; trial < 1000; ++trial) {
        gen.fill(arrA, any_domain);
        gen.fill(arrSrc, any_domain);

        V a   = raze::vx::load<V>(arrA);
        V src = raze::vx::load<V>(arrSrc);

        Mask m = rtts::simd::make_random_mask<Mask>();

        auto r1 = raze::vx::neg(a);
        auto r2 = raze::vx::neg[m](a);
        auto r3 = raze::vx::neg[m, src](a);

        RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return T(-arrA[i]); });
        RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) { return m[i] ? T(-arrA[i]) : T(0); });
        RTTS_ALL_VALIDATE_VECTOR(r3, [&](auto i) { return m[i] ? T(-arrA[i]) : arrSrc[i]; });
    }

    gen.fill(arrA, any_domain);
    for (size_t i = 0; i < N; ++i) arrSrc[i] = T(0);

    V a = raze::vx::load<V>(arrA);
    V zeros = raze::vx::load<V>(arrSrc);
    auto r_double_neg = raze::vx::neg(raze::vx::neg(a));
    RTTS_ALL_VALIDATE_VECTOR(r_double_neg, [&](auto i) { return arrA[i]; });
};

RAZE_TEST_NAMESPACE_END