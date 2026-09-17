#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::slide_right", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    constexpr size_t N = V::size();

    alignas(64) T arr[N];
    std::iota(arr, arr + N, T(1));

    V v = raze::vx::load<V>(arr);

    for (size_t sh = 0; sh < N; ++sh) {
        auto r = raze::vx::slide_right(v, sh);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == ((i >= sh) ? T(arr[i - sh]) : T(0)));
    }

    [&] <raze::sizetype... I> (std::integer_sequence<raze::sizetype, I...>) {
        ([&] {
            if constexpr (!(N > 16 && (I != N && I != N / 2 && I != 0 && I > 8))) {
                auto r = raze::vx::slide_right(v, std::integral_constant<raze::sizetype, I>{});

                for (size_t i = 0; i < N; ++i)
                    RTTS_EXPECT(r[i] == ((i >= I) ? T(arr[i - I]) : T(0)));
            }
        }(), ...);
    }(std::make_integer_sequence<raze::sizetype, N + 1>{});
};

RAZE_TEST_NAMESPACE_END