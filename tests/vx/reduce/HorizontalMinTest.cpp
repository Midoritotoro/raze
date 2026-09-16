#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::horizontal_min", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();

    alignas(64) T arr[N];
    alignas(64) T fallback[N];

    for (size_t i = 0; i < N; ++i) {
        fallback[i] = T(i + 1);
        arr[i] = T((i % 3 == 0) ? -(i + 5) : (i + 1));
    }

    V v = raze::vx::load<V>(arr);
    V fbk = raze::vx::load<V>(fallback);

    {
        auto result = raze::vx::horizontal_min(v);
        T expected = *std::min_element(arr, arr + N);

        RTTS_EXPECT(result == expected);
    }

    for (size_t iteration = 0; iteration < 1000; ++iteration) {
        (void)iteration;

        Mask mask = rtts::simd::make_random_mask<Mask>();

        {
            auto result = raze::vx::horizontal_min[fbk, mask](v);

            T expected = mask[0] ? arr[0] : fallback[0];

            for (size_t i = 1; i < N; ++i) {
                T value = mask[i] ? arr[i] : fallback[i];
                expected = std::min(expected, value);
            }

            RTTS_EXPECT(result == expected);
        }

        {
            auto result = raze::vx::horizontal_min[mask](v);

            T expected = mask[0] ? arr[0] : T(0);

            for (size_t i = 1; i < N; ++i) {
                T value = mask[i] ? arr[i] : T(0);
                expected = std::min(expected, value);
            }

            RTTS_EXPECT(result == expected);
        }
    }
};

RAZE_TEST_NAMESPACE_END
