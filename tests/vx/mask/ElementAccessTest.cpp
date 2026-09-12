#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::simd_mask<T> element access", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    Mask m(false);

    m[0] = true;
    m[N - 1] = true;

    RTTS_EXPECT(m[0]);
    RTTS_EXPECT(m[N - 1]);

    for (size_t i = 1; i + 1 < N; ++i)
        RTTS_EXPECT(!m[i]);
};