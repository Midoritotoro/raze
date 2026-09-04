#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RTTS_CASE_TPL("raze::vx::is_greater_equal", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    constexpr size_t N = V::size();

    alignas(64) T arrA[N], arrB[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T(N - i);
    }

    V a = raze::vx::load<V>(arrA);
    V b = raze::vx::load<V>(arrB);

    RTTS_ALL_VALIDATE(a >= b, [=](auto i) { return arrA[i] >= arrB[i]; });
};
