#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <bit>
#include <cmath>
#include <algorithm>

RAZE_TEST_NAMESPACE_BEGIN
 
template <class V, raze::u8 Op>
V call_tlogic(V a, V b, V c) {
    return raze::vx::ternarylogic(a, b, c, std::integral_constant<raze::u8, Op>{});
}

template <class V, raze::u8 Op>
V call_tlogic_masked(V a, V b, V c, typename V::mask_type m, V src) {
    return raze::vx::ternarylogic[m, src](a, b, c, std::integral_constant<raze::u8, Op>{});
}

template <class V>
raze_never_inline V dispatch_tlogic(raze::u8 op, V a, V b, V c) {
#define RAZE_TLOGIC_CASE(n) case (n): return call_tlogic<V, (n)>(a, b, c)
#define RAZE_TLOGIC_CASE_16(base)                                    \
    RAZE_TLOGIC_CASE((base) +  0); RAZE_TLOGIC_CASE((base) +  1);    \
    RAZE_TLOGIC_CASE((base) +  2); RAZE_TLOGIC_CASE((base) +  3);    \
    RAZE_TLOGIC_CASE((base) +  4); RAZE_TLOGIC_CASE((base) +  5);    \
    RAZE_TLOGIC_CASE((base) +  6); RAZE_TLOGIC_CASE((base) +  7);    \
    RAZE_TLOGIC_CASE((base) +  8); RAZE_TLOGIC_CASE((base) +  9);    \
    RAZE_TLOGIC_CASE((base) + 10); RAZE_TLOGIC_CASE((base) + 11);    \
    RAZE_TLOGIC_CASE((base) + 12); RAZE_TLOGIC_CASE((base) + 13);    \
    RAZE_TLOGIC_CASE((base) + 14); RAZE_TLOGIC_CASE((base) + 15)
    switch (op) {
        RAZE_TLOGIC_CASE_16(  0); RAZE_TLOGIC_CASE_16( 16);
        RAZE_TLOGIC_CASE_16( 32); RAZE_TLOGIC_CASE_16( 48);
        RAZE_TLOGIC_CASE_16( 64); RAZE_TLOGIC_CASE_16( 80);
        RAZE_TLOGIC_CASE_16( 96); RAZE_TLOGIC_CASE_16(112);
        RAZE_TLOGIC_CASE_16(128); RAZE_TLOGIC_CASE_16(144);
        RAZE_TLOGIC_CASE_16(160); RAZE_TLOGIC_CASE_16(176);
        RAZE_TLOGIC_CASE_16(192); RAZE_TLOGIC_CASE_16(208);
        RAZE_TLOGIC_CASE_16(224); RAZE_TLOGIC_CASE_16(240);
    }
#undef RAZE_TLOGIC_CASE_16
#undef RAZE_TLOGIC_CASE
}

template <class V>
raze_never_inline V dispatch_tlogic_masked(
    raze::u8 op, V a, V b, V c, typename V::mask_type m, V src)
{
#define RAZE_TLOGIC_CASE(n) case (n): return call_tlogic_masked<V, (n)>(a, b, c, m, src)
#define RAZE_TLOGIC_CASE_16(base)                                    \
    RAZE_TLOGIC_CASE((base) +  0); RAZE_TLOGIC_CASE((base) +  1);    \
    RAZE_TLOGIC_CASE((base) +  2); RAZE_TLOGIC_CASE((base) +  3);    \
    RAZE_TLOGIC_CASE((base) +  4); RAZE_TLOGIC_CASE((base) +  5);    \
    RAZE_TLOGIC_CASE((base) +  6); RAZE_TLOGIC_CASE((base) +  7);    \
    RAZE_TLOGIC_CASE((base) +  8); RAZE_TLOGIC_CASE((base) +  9);    \
    RAZE_TLOGIC_CASE((base) + 10); RAZE_TLOGIC_CASE((base) + 11);    \
    RAZE_TLOGIC_CASE((base) + 12); RAZE_TLOGIC_CASE((base) + 13);    \
    RAZE_TLOGIC_CASE((base) + 14); RAZE_TLOGIC_CASE((base) + 15)
    switch (op) {
        RAZE_TLOGIC_CASE_16(  0); RAZE_TLOGIC_CASE_16( 16);
        RAZE_TLOGIC_CASE_16( 32); RAZE_TLOGIC_CASE_16( 48);
        RAZE_TLOGIC_CASE_16( 64); RAZE_TLOGIC_CASE_16( 80);
        RAZE_TLOGIC_CASE_16( 96); RAZE_TLOGIC_CASE_16(112);
        RAZE_TLOGIC_CASE_16(128); RAZE_TLOGIC_CASE_16(144);
        RAZE_TLOGIC_CASE_16(160); RAZE_TLOGIC_CASE_16(176);
        RAZE_TLOGIC_CASE_16(192); RAZE_TLOGIC_CASE_16(208);
        RAZE_TLOGIC_CASE_16(224); RAZE_TLOGIC_CASE_16(240);
    }
#undef RAZE_TLOGIC_CASE_16
#undef RAZE_TLOGIC_CASE
}

RTTS_CASE_TPL("raze::vx::ternarylogic", rtts::simd::all_simd_infos)
<class SimdInfo> (rtts::type<SimdInfo>) {
    using V    = typename SimdInfo::type;
    using T    = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrA[N], arrB[N], arrC[N], arrSrc[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i]   = T(i + 1);
        arrB[i]   = T((i + 1) * 3);
        arrC[i]   = T((i + 1) * 7);
        arrSrc[i] = T(100 + i);
    }

    V a   = raze::vx::load<V>(arrA);
    V b   = raze::vx::load<V>(arrB);
    V c   = raze::vx::load<V>(arrC);
    V src = raze::vx::load<V>(arrSrc);

    using U = typename raze::IntegerForSizeof<T>::Unsigned;

    auto scalar_eval = [&](auto i, raze::u8 op) -> T {
        U uA = raze::math::bit_cast<U>(arrA[i]);
        U uB = raze::math::bit_cast<U>(arrB[i]);
        U uC = raze::math::bit_cast<U>(arrC[i]);
        U result = 0;
        for (size_t b = 0; b < sizeof(T) * 8; ++b) {
            bool xa = (uA >> b) & 1;
            bool xb = (uB >> b) & 1;
            bool xc = (uC >> b) & 1;
            raze::u8 idx = (raze::u8(xa) << 2) | (raze::u8(xb) << 1) | raze::u8(xc);
            if ((op >> idx) & 1) {
                result |= (U(1) << b);
            }
        }
        return raze::math::bit_cast<T>(result);
    };

    const int mask_iters =
        std::min(static_cast<int>(std::pow(2, N)), 100);

    for (int op_i = 0; op_i <= 255; ++op_i) {
        raze::u8 op = static_cast<raze::u8>(op_i);

        for (int k = 0; k < mask_iters; ++k) {
            Mask m = rtts::simd::make_random_mask<Mask>();

            V r1 = dispatch_tlogic<V>(op, a, b, c);
            RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return scalar_eval(i, op); });

            V r2 = dispatch_tlogic_masked<V>(op, a, b, c, m, src);
            RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) {
                return m[i] ? scalar_eval(i, op) : arrSrc[i];
            });
        }
    }
};

RAZE_TEST_NAMESPACE_END