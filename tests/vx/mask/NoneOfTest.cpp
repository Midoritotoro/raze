#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::none_of", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto scalar_impl = [] (auto mask) { 
        for (auto i = 0; i < mask.size(); ++i)
            if (mask[i]) return false;

        return true;
    };

    auto scalar_with_control_impl = [] (auto mask, auto control) { 
        for (auto i = 0; i < mask.size(); ++i)
            if (control[i] && mask[i]) 
                return false;

        return true;
    };

    {
        Mask m(false);
        RTTS_EXPECT(scalar_impl(m) == raze::vx::none_of(m));11
    }

    {
        Mask m(true);
        RTTS_EXPECT(scalar_impl(m) == raze::vx::none_of(m));
    }

    for (raze::sizetype i = 0; i < 100; ++i) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        Mask control = rtts::simd::make_random_mask<Mask>();

        auto r1 = raze::vx::none_of(m);
        auto r2 = raze::vx::none_of[control](m);
        auto r1_scalar = scalar_impl(m);
        auto r2_scalar = scalar_with_control_impl(m, control);

        RTTS_EXPECT(r1 == r1_scalar);
        RTTS_EXPECT(r2 == r2_scalar);
    }
};