#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::broadcast", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrSrc[i] = T(100 + i);
    }

    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    const T value = T(42);

    auto r1 = raze::vx::broadcast<V>(value);
    auto r2 = raze::vx::broadcast<V>[m](value);
    auto r3 = raze::vx::broadcast<V>[m, src](value);

    RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto) { return value; });
    RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) { return m[i] ? value : T(0); });
    RTTS_ALL_VALIDATE_VECTOR(r3, [&](auto i) { return m[i] ? value : arrSrc[i]; });
};
 
RAZE_TEST_NAMESPACE_END
