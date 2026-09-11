#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <random>

RTTS_CASE_TPL("raze::vx::find_first_set", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_find_first = [=](const Mask& m) {
        for (size_t i = 0; i < N; ++i)
            if (m[i]) return i;
        return N;
    };

    auto scalar_find_first_with_control = [=](const Mask& m, const Mask& control) {
        for (size_t i = 0; i < N; ++i)
            if (control[i] && m[i]) return i;
        return N;
    };

    {
        Mask m(false);
        RTTS_EXPECT(raze::vx::find_first_set(m) == N);
    }

    {
        Mask m(true);
        RTTS_EXPECT(raze::vx::find_first_set(m) == 0);
    }

    for (size_t i = 0; i < N; ++i) {
        Mask m(false);
        m[i] = true;
        RTTS_EXPECT(raze::vx::find_first_set(m) == i);
    }

    {
        Mask m(false);
        for (size_t i = 0; i < N; ++i)
            m[i] = (i % 2 == 0);
        RTTS_EXPECT(raze::vx::find_first_set(m) == 0);
    }

    for (int iter = 0; iter < 100; ++iter) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        Mask control = rtts::simd::make_random_mask<Mask>();

        RTTS_EXPECT(raze::vx::find_first_set(m) == scalar_find_first(m));
        RTTS_EXPECT(raze::vx::find_first_set[control](m) == scalar_find_first_with_control(m, control));
    }
};
