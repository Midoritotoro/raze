#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::rotate_right", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    constexpr size_t N = V::size();

    T arr[N];
    std::iota(arr, arr + N, T(1));

    V v = raze::vx::load<V>(arr);

    for (size_t sh = 0; sh < N; ++sh) {
        auto result = raze::vx::rotate_right(v, sh);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(result[i] == arr[(i + N - sh) % N]);
    }

    [&] <size_t... I>(std::index_sequence<I...>) {
        ([&] {
            auto result = raze::vx::rotate_right(v, std::integral_constant<size_t, I>{});

            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(result[i] == arr[(i + N - I) % N]);
        }(), ...);
    }(std::make_index_sequence<N>{});
};

RAZE_TEST_NAMESPACE_END
