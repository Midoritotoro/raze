#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::any_of", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_impl = [] (auto mask) { 
        for (auto i = 0; i < mask.size(); ++i)
            if (mask[i]) return true;

        return false;
    };

    auto scalar_with_control_impl = [] (auto mask, auto control) { 
        for (auto i = 0; i < mask.size(); ++i)
            if (control[i] && mask[i]) return true;

        return false;
    };

    for (raze::sizetype i = 0; i < 100; ++i) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        Mask control = rtts::simd::make_random_mask<Mask>();

        auto r1 = raze::vx::any_of(m);
        auto r2 = raze::vx::any_of[control](m);

        RTTS_EXPECT(r1 == scalar_impl(m));
        RTTS_EXPECT(r2 == scalar_with_control_impl(m, control));
    }
};