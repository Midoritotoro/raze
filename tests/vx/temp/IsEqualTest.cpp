#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

raze_disable_warning_msvc(4805)

[[maybe_unused]] static bool const rtts_case_3 = ::rtts::detail::test_captures<rtts::simd::all_types>{ "Check SIMD comparisons" } + []
<typename Simd>(rtts::type<Simd>) 
{
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

    RTTS_ALL_EQUAL(a == a, V(true));
};