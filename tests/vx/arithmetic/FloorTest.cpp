#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::floor", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    rtts::vx_fp::test_unary_exact<V>(raze::vx::floor, [](T x) { return std::floor(x); }, rtts::vx_fp::round_values<T>(), 0x26001ULL);
};

RTTS_CASE("raze::vx::floor non-native and constraints") {
    auto ref = [](auto x) { return std::floor(x); };
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::floor, ref, rtts::vx_fp::round_values<raze::f32>(), 0x26002ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::floor, ref, rtts::vx_fp::round_values<raze::f64>(), 0x26003ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::floor, ref, rtts::vx_fp::round_values<raze::f32>(), 0x26004ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::floor, ref, rtts::vx_fp::round_values<raze::f64>(), 0x26005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::floor), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

