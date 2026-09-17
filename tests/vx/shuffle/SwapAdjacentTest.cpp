#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::swap_adjacent", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    constexpr size_t N = V::size();
    T arr[N];

    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    V v = raze::vx::load<V>(arr);

    [&] <raze::sizetype... GroupSize> (std::integer_sequence<raze::sizetype, GroupSize...>) {
        ([&] {
            if constexpr (GroupSize >= 2) {
                 // ...
            }
        }(), ...);
    }(std::make_integer_sequence<raze::sizetype, N + 1>{});
};

RAZE_TEST_NAMESPACE_END