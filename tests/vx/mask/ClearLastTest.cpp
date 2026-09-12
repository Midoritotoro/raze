#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::clear_last", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    {
        Mask m_empty(false);
        Mask m_empty_cleared = raze::vx::clear_last(m_empty);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!m_empty_cleared[i]);
    }

    {
        Mask m(false);
        m[0] = true;
        auto r = raze::vx::clear_last(m);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!r[i]);
    }

    {
        Mask m(false);
        m[N - 1] = true;
        auto r = raze::vx::clear_last(m);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!r[i]);
    }

    if constexpr (N > 4) {
        Mask m(false);
        m[0] = true;
        m[2] = true;
        m[N - 2] = true;

        auto r = raze::vx::clear_last(m);

        RTTS_EXPECT(r[0]);
        RTTS_EXPECT(r[2]);
        RTTS_EXPECT(!r[N - 2]);

        for (size_t i = 0; i < N; ++i) {
            if (i != 0 && i != 2 && i != N - 2)
                RTTS_EXPECT(!r[i]);
        }
    }

    {
        Mask m(true);
        auto r = raze::vx::clear_last(m);

        RTTS_EXPECT(!r[N - 1]);

        for (size_t i = 0; i + 1 < N; ++i)
            RTTS_EXPECT(r[i]);
    }
};