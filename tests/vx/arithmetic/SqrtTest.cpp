#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::sqrt", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    rtts::vx_fp::test_unary_exact<V>(raze::vx::sqrt, [](T x) { return std::sqrt(x); }, rtts::vx_fp::sqrt_rcp_values<T>(), 0x21001ULL);
};

RTTS_CASE("raze::vx::sqrt non-native and constraints") {
    auto ref = [](auto x) { return std::sqrt(x); };
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::sqrt, ref, rtts::vx_fp::sqrt_rcp_values<raze::f32>(), 0x21002ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::sqrt, ref, rtts::vx_fp::sqrt_rcp_values<raze::f64>(), 0x21003ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::sqrt, ref, rtts::vx_fp::sqrt_rcp_values<raze::f32>(), 0x21004ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::sqrt, ref, rtts::vx_fp::sqrt_rcp_values<raze::f64>(), 0x21005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::sqrt), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

