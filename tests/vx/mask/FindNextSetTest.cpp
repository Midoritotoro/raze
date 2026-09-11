#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::find_next_set", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_find_next = [=](const Mask& m, size_t pos) {
        for (size_t i = pos + 1; i < N; ++i)
            if (m[i]) return i;
        return N;
    };

    {
        Mask m(false);
        for (size_t pos = 0; pos < N; ++pos) {
            RTTS_EXPECT(raze::vx::find_next_set(m, pos) == N);
        }
    }

    {
        Mask m(true);
        for (size_t pos = 0; pos < N; ++pos) {
            auto expected = (pos + 1 < N) ? pos + 1 : N;
            RTTS_EXPECT(raze::vx::find_next_set(m, pos) == expected);
        }
    }

    for (size_t bit = 0; bit < N; ++bit) {
        Mask m(false);
        m[bit] = true;
        for (size_t pos = 0; pos < N; ++pos) {
            RTTS_EXPECT(raze::vx::find_next_set(m, pos) == scalar_find_next(m, pos));
        }
    }

    {
        Mask m(false);
        for (size_t i = 0; i < N; ++i)
            m[i] = (i % 2 == 0);
        for (size_t pos = 0; pos < N; ++pos)
            RTTS_EXPECT(raze::vx::find_next_set(m, pos) == scalar_find_next(m, pos));
    }

    std::mt19937 rng(12345);
    for (int iter = 0; iter < 100; ++iter) {
        Mask m(false);
        for (size_t i = 0; i < N; ++i)
            m[i] = (rng() & 1);
        for (size_t pos = 0; pos < N; ++pos)
            RTTS_EXPECT(raze::vx::find_next_set(m, pos) == scalar_find_next(m, pos));
    }
};
