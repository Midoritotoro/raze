#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <numeric>
#include <functional>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::horizontal_sum", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    using R = raze::vx::reduce_type<T>;

    constexpr size_t N = V::size();

    T arr[N];
    T fallback[N];

    std::iota(arr, arr + N, T(1));
    std::iota(fallback, fallback + N, T(42));

    V v = raze::vx::load<V>(arr);
    V fbk = raze::vx::load<V>(fallback);

    {
        auto result = raze::vx::hsum(v);

        R expected = std::accumulate(arr, arr + N, R(0), std::plus{});
        RTTS_EXPECT(result == expected);
    }

    for (size_t iteration = 0; iteration < 1000; ++iteration) {
        (void)iteration;

        Mask mask = rtts::simd::make_random_mask<Mask>();

        {
            auto result = raze::vx::horizontal_sum[mask, fbk](v);
            R expected = R(0);

            for (size_t i = 0; i < N; ++i)
                expected += mask[i] ? static_cast<R>(arr[i]) : static_cast<R>(fallback[i]);

            RTTS_EXPECT(result == expected);
        }

        {
            auto result = raze::vx::horizontal_sum[mask](v);
            R expected = R(0);

            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) expected += static_cast<R>(arr[i]);
            }

            RTTS_EXPECT(result == expected);
        }
    }
};

RAZE_TEST_NAMESPACE_END
