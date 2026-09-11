#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::find_last_not_set", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_find_last_not = [=](const Mask& m) {
        for (size_t i = N; i > 0; --i)
            if (!m[i - 1]) return N - i;
        return N;
    };

    auto scalar_find_last_not_with_control = [=](const Mask& m, const Mask& control) {
        for (size_t i = N; i > 0; --i)
            if (control[i - 1] && !m[i - 1]) return N - i;
        return N;
    };

    {
        Mask m(true);
        RTTS_EXPECT(raze::vx::find_last_not_set(m) == N);
    }

    {
        Mask m(false);
        RTTS_EXPECT(raze::vx::find_last_not_set(m) == 0);
    }

    for (size_t i = 0; i < N; ++i) {
        Mask m(true);
        m[i] = false;
        RTTS_EXPECT(raze::vx::find_last_not_set(m) == (N - i - 1));
    }

    for (int iter = 0; iter < 100; ++iter) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        Mask control = rtts::simd::make_random_mask<Mask>();

        RTTS_EXPECT(raze::vx::find_last_not_set(m) == scalar_find_last_not(m));
        RTTS_EXPECT(raze::vx::find_last_not_set[control](m) == scalar_find_last_not_with_control(m, control));
    }
};
