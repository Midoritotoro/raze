#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::is_nan", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    rtts::vx_fp::test_unary_predicate<V>(raze::vx::is_nan, [](auto x) { return rtts::vx_fp::ref_is_nan(x); }, 0x11001ULL);
};

RTTS_CASE("raze::vx::is_nan non-native and constraints") {
    auto ref = [](auto x) { return rtts::vx_fp::ref_is_nan(x); };
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::is_nan, ref, 0x11002ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::is_nan, ref, 0x11003ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::is_nan, ref, 0x11004ULL);
    rtts::vx_fp::test_unary_predicate<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::is_nan, ref, 0x11005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::is_nan), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

