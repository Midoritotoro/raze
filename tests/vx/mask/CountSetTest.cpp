#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::count_set", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_count = [=](const Mask& m) {
        size_t count = 0;
        for (size_t i = 0; i < N; ++i)
            if (m[i]) ++count;
        return count;
    };

    auto scalar_count_with_control = [=](const Mask& m, const Mask& control) {
        size_t count = 0;
        for (size_t i = 0; i < N; ++i)
            if (control[i] && m[i]) ++count;
        return count;
    };

    {
        Mask m(false);
        RTTS_EXPECT(raze::vx::count_set(m) == 0);
    }

    {
        Mask m(true);
        RTTS_EXPECT(raze::vx::count_set(m) == N);
    }

    for (size_t i = 0; i < N; ++i) {
        Mask m(false);
        m[i] = true;
        RTTS_EXPECT(raze::vx::count_set(m) == 1);
    }

    {
        Mask m(false);
        for (size_t i = 0; i < N; ++i)
            m[i] = (i % 2 == 0);
        RTTS_EXPECT(raze::vx::count_set(m) == (N + 1) / 2);
    }

    for (int iter = 0; iter < 100; ++iter) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        Mask control = rtts::simd::make_random_mask<Mask>();

        RTTS_EXPECT(raze::vx::count_set(m) == scalar_count(m));
        RTTS_EXPECT(raze::vx::count_set[control](m) == scalar_count_with_control(m, control));
    }
};
