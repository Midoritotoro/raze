#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::first_n", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    {
        auto m = raze::vx::first_n(0, raze::options::as(Mask{}));
        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!m[i]);
    }

    {
        auto m = raze::vx::first_n(N, raze::options::as(Mask{}));
        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(m[i]);
    }

    {
        for (size_t k = 0; k <= N; ++k) {
            auto m = raze::vx::first_n(k, raze::options::as(Mask{}));
            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(m[i] == (i < k));
        }
    }

    {
        std::mt19937 rng(98765);
        for (int iter = 0; iter < 500; ++iter) {
            size_t k = rng() % (N + 1);
            auto m = raze::vx::first_n(k, raze::options::as(Mask{}));
            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(m[i] == (i < k));
        }
    }
};