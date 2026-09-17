#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::slide_right_merge", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    constexpr size_t N = V::size();

    T arr1[N];
    T arr2[N];

    std::iota(arr1, arr1 + N, T(1));
    std::iota(arr2, arr2 + N, T(N + 1));

    V v1 = raze::vx::load<V>(arr1);
    V v2 = raze::vx::load<V>(arr2);

    for (size_t sh = 0; sh < N; ++sh) {
        auto r = raze::vx::slide_right_merge(v1, v2, sh);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == ((i >= sh) ? T(arr1[i - sh]) : T(arr2[N - sh + i])));
    }

    [&] <raze::sizetype... I> (std::integer_sequence<raze::sizetype, I...>) {
        ([&] {
            auto r = raze::vx::slide_right_merge(v1, v2, std::integral_constant<raze::sizetype, I>{});

            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(r[i] == ((i >= I) ? T(arr1[i - I]) : T(arr2[N - I + i])));
        }(), ...);
    }(std::make_integer_sequence<raze::sizetype, N>{});
};

RAZE_TEST_NAMESPACE_END