#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::round", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    rtts::vx_fp::test_unary_exact<V>(raze::vx::round, [](T x) { return std::round(x); }, rtts::vx_fp::round_values<T>(), 0x29001ULL);
};

RTTS_CASE("raze::vx::round non-native and constraints") {
    auto ref = [](auto x) { return std::round(x); };
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::round, ref, rtts::vx_fp::round_values<raze::f32>(), 0x29002ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::round, ref, rtts::vx_fp::round_values<raze::f64>(), 0x29003ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::round, ref, rtts::vx_fp::round_values<raze::f32>(), 0x29004ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::round, ref, rtts::vx_fp::round_values<raze::f64>(), 0x29005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::round), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

