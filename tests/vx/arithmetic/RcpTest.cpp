#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

namespace {
    template <class V>
    void test_rcp_vector() {
        using T = typename V::value_type;
        using Mask = typename V::mask_type;
        constexpr size_t N = V::size();
        const double raw_tol = (sizeof(T) == 4) ? 2.0e-3 : 2.0e-4;

        rtts::vx_fp::test_unary_exact<V>(
            raze::vx::rcp,
            [](T x) { return T(1) / x; },
            rtts::vx_fp::sqrt_rcp_values<T>(),
            0x23001ULL);

        alignas(alignof(V)) T arrA[N];
        alignas(alignof(V)) T arrSrc[N];
        rtts::random::generator<T> gen(0x23002ULL + sizeof(T) * 41 + N);
        const auto nz_dom = rtts::random::domain<T>::non_zero();
        const auto std_dom = rtts::random::domain<T>::standard();

        for (size_t trial = 0; trial < 300; ++trial) {
            gen.fill(arrA, nz_dom);
            gen.fill(arrSrc, std_dom);
            const Mask m = rtts::simd::make_random_mask<Mask>();

            const V a = raze::vx::load<V>(arrA);
            const V src = raze::vx::load<V>(arrSrc);

            const V r1 = raze::vx::rcp[raze::vx::raw](a);
            const V r2 = raze::vx::rcp[raze::vx::raw][m](a);
            const V r3 = raze::vx::rcp[raze::vx::raw][m, src](a);
            const V r4 = raze::vx::rcp[m, src][raze::vx::raw](a);

            for (size_t i = 0; i < N; ++i) {
                const T erc = T(1) / arrA[i];
                RTTS_EXPECT(rtts::vx_fp::approx_match(r1[i], erc, raw_tol));
                RTTS_EXPECT(m[i] ? rtts::vx_fp::approx_match(r2[i], erc, raw_tol) : rtts::vx_fp::bits_match(r2[i], T(0)));
                RTTS_EXPECT(m[i] ? rtts::vx_fp::approx_match(r3[i], erc, raw_tol) : rtts::vx_fp::bits_match(r3[i], arrSrc[i]));
                RTTS_EXPECT(m[i] ? rtts::vx_fp::approx_match(r4[i], erc, raw_tol) : rtts::vx_fp::bits_match(r4[i], arrSrc[i]));
            }
        }
    }
}

RTTS_CASE_TPL("raze::vx::rcp", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    test_rcp_vector<V>();
};

RTTS_CASE("raze::vx::rcp non-native and constraints") {
    test_rcp_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>();
    test_rcp_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>();
    test_rcp_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>();
    test_rcp_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>();
    static_assert(!std::is_invocable_v<decltype(raze::vx::rcp), raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

