#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::is_not_finite", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    rtts::vx_fp::test_unary_predicate<V>(raze::vx::is_not_finite, [](auto x) { return !rtts::vx_fp::ref_is_finite(x); }, 0x14001ULL);
};

RTTS_CASE("raze::vx::is_not_finite non-native and constraints") {
    auto ref = [](auto x) { return !rtts::vx_fp::ref_is_finite(x); };
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::is_not_finite, ref, 0x14002ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::is_not_finite, ref, 0x14003ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::is_not_finite, ref, 0x14004ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::is_not_finite, ref, 0x14005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::is_not_finite), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

