#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::bit_shl", rtts::simd::all_simd_infos)
< class Simd > (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    if constexpr (!std::is_integral_v<T>) {
        RTTS_EXPECT(true);
        return;
    }

    alignas(alignof(V)) T arrA[N], arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T((i * 0x5A5A) ^ (i + 3));
        arrSrc[i] = T(100 + i);
    }

    arrA[0] = T(-1);
    if constexpr (N > 1) arrA[N - 1] = T(255);

    V a = raze::vx::load<V>(arrA);
    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    if constexpr (std::integral<T>) {
        for (size_t sh = 0; sh < N; ++sh) {
            auto scalar_eval = [=](size_t i) {
                if (sh < sizeof(T) * 8)
                    return T(arrA[i] << sh);
                else
                    return T(0);
                };

            auto r1 = raze::vx::bit_shl(a, sh);
            auto r2 = raze::vx::bit_shl[m](a, sh);
            auto r3 = raze::vx::bit_shl[m, src](a, sh);

            RTTS_ALL_VALIDATE_BITS(r1, [&](auto i) { return scalar_eval(i); });
            RTTS_ALL_VALIDATE_BITS(r2, [&](auto i) { return m[i] ? scalar_eval(i) : T(0); });
            RTTS_ALL_VALIDATE_BITS(r3, [&](auto i) { return m[i] ? scalar_eval(i) : arrSrc[i]; });
        }
    }
};