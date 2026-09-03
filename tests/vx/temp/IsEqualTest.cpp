#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

raze_disable_warning_msvc(4805)

//inline const char* isa_to_string(raze::arch::ISA isa) {
//    if (isa == raze::arch::ISA::None) return "None";
//    if (isa == raze::arch::ISA::SSE2) return "SSE2";
//    if (isa == raze::arch::ISA::AVX) return "AVX";
//    if (isa == raze::arch::ISA::AVX2) return "AVX2";
//    if (isa == raze::arch::ISA::AVX512F) return "AVX512F";
//    return "UNKNOWN";
//}
//
//RTTS_CASE_TPL("Is equal test", rtts::simd::fp_types)
//< typename Simd > (rtts::type<Simd>)
//{
//    std::cout << "----------------------------------------\n";
//
//
//    std::cout << "target_isa() returned: " << isa_to_string(raze::vx::target_isa()) << "\n";
//    std::cout << "----------------------------------------\n";
//
//    RTTS_EXPECT(true);
//};


RTTS_CASE_TPL("Is equal test", rtts::simd::all_simd_infos)
< typename Simd > (rtts::type<Simd>)
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
