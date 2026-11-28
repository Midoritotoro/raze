#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::trunc", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    rtts::vx_fp::test_unary_exact<V>(raze::vx::trunc, [](T x) { return std::trunc(x); }, rtts::vx_fp::round_values<T>(), 0x28001ULL);
};

RTTS_CASE("raze::vx::trunc non-native and constraints") {
    auto ref = [](auto x) { return std::trunc(x); };
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>(raze::vx::trunc, ref, rtts::vx_fp::round_values<raze::f32>(), 0x28002ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>(raze::vx::trunc, ref, rtts::vx_fp::round_values<raze::f64>(), 0x28003ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>(raze::vx::trunc, ref, rtts::vx_fp::round_values<raze::f32>(), 0x28004ULL);
    rtts::vx_fp::test_unary_exact<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>(raze::vx::trunc, ref, rtts::vx_fp::round_values<raze::f64>(), 0x28005ULL);
    static_assert(!std::is_invocable_v<decltype(raze::vx::trunc), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

