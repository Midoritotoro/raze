#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::is_subnormal", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    rtts::vx_fp::test_unary_predicate<V>(raze::vx::is_subnormal, [](auto x) { return rtts::vx_fp::ref_is_subnormal(x); }, 0x16001ULL);
};

RTTS_CASE("raze::vx::is_subnormal non-native and constraints") {
    auto ref = [](auto x) { return rtts::vx_fp::ref_is_subnormal(x); };
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::is_subnormal, ref, 0x16002ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::is_subnormal, ref, 0x16003ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::is_subnormal, ref, 0x16004ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::is_subnormal, ref, 0x16005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::is_subnormal), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

