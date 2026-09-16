#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::select", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();

    T a_arr[N];
    T b_arr[N];

    for (size_t i = 0; i < N; ++i) {
        a_arr[i] = T(i + 1);
        b_arr[i] = T(200 + i);
    }

    V a = raze::vx::load<V>(a_arr);
    V b = raze::vx::load<V>(b_arr);

    auto test_mask = [&](Mask mask) {
        {
            auto r = raze::vx::select[mask, b](a);
            RTTS_ALL_VALIDATE_BITS(r, [&](auto i) { return mask[i] ? a_arr[i] : b_arr[i]; });
        }

        {
            auto r = raze::vx::select[mask](a);
            RTTS_ALL_VALIDATE_BITS(r, [&](auto i) { return mask[i] ? a_arr[i] : T(0); });
        }
    };

    test_mask(Mask(true));
    test_mask(Mask(false));
    test_mask(rtts::simd::make_alternating_mask<Mask>());

    for (size_t i = 0; i < 100; ++i)
        test_mask(rtts::simd::make_random_mask<Mask>());
};

RAZE_TEST_NAMESPACE_END
