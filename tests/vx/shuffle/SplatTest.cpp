#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::splat", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    constexpr size_t N = V::size();

    T arr[N];

    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    V v = raze::vx::load<V>(arr);

    [&] <raze::sizetype... I> (std::integer_sequence<raze::sizetype, I...>) {
        ([&] {
            auto r = raze::vx::splat<I>(v);

            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(r[i] == arr[I]);
        }(), ...);
    }(std::make_integer_sequence<raze::sizetype, N>{});
};

RAZE_TEST_NAMESPACE_END