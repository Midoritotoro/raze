#include <tests/rts/rts.h>
#include <raze/vx/Simd.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::math::fms", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();

    T A[N], B[N], C[N], S[N];
    for (size_t i = 0; i < N; ++i) {
        const T x = T(i + 1);
        A[i] = x;
        B[i] = x * 3;
        C[i] = x * 7;
        S[i] = T(100 + i);
    }

    const V a = raze::vx::load<V>(A);
    const V b = raze::vx::load<V>(B);
    const V c = raze::vx::load<V>(C);
    const V src = raze::vx::load<V>(S);

    Mask mask;
    for (size_t i = 0; i < N; ++i)
        mask[i] = !(i & 1);

    const auto r       = raze::math::fms(a, b, c);
    const auto r_merge = raze::math::fms[mask, src](a, b, c);
    const auto r_zero  = raze::math::fms[mask](a, b, c);

    const auto reference_fms = [](T x, T y, T z) -> T {
        return static_cast<T>(x * y - z);
    };

    for (size_t i = 0; i < N; ++i) {
        const T expected = reference_fms(A[i], B[i], C[i]);

        RTTS_EXPECT(r[i] == expected);
        RTTS_EXPECT(r_merge[i] == (mask[i] ? expected : S[i]));
        RTTS_EXPECT(r_zero[i] == (mask[i] ? expected : T(0)));

        const T masked_expected = mask[i] ? expected : S[i];
        auto raze_scalar = raze::math::fms[bool(mask[i]), T(S[i])](A[i], B[i], C[i]);
        RTTS_EXPECT(raze_scalar == masked_expected);
    }
};


RAZE_TEST_NAMESPACE_END