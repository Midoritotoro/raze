#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

namespace {
    template <class V>
    void test_copysign_vector() {
        using T = typename V::value_type;
        using Mask = typename V::mask_type;
        constexpr size_t N = V::size();

        const auto vals = rtts::vx_fp::round_values<T>();
        alignas(alignof(V)) T arrA[N];
        alignas(alignof(V)) T arrB[N];
        alignas(alignof(V)) T arrSrc[N];

        auto check = [&](const Mask& m) {
            const V a = raze::vx::load<V>(arrA);
            const V b = raze::vx::load<V>(arrB);
            const V src = raze::vx::load<V>(arrSrc);

            const V r1 = raze::vx::copysign(a, b);
            const V r2 = raze::vx::copysign[m](a, b);
            const V r3 = raze::vx::copysign[m, src](a, b);
            const V r4 = raze::vx::copysign(a, arrB[0]);
            const V r5 = raze::vx::copysign(arrA[0], b);
            const V r6 = raze::vx::copysign[m, src](a, arrB[0]);
            const V r7 = raze::vx::copysign[m, src](arrA[0], b);

            for (size_t i = 0; i < N; ++i) {
                const T e1 = std::copysign(arrA[i], arrB[i]);
                const T e4 = std::copysign(arrA[i], arrB[0]);
                const T e5 = std::copysign(arrA[0], arrB[i]);

                RTTS_EXPECT(rtts::vx_fp::bits_match(r1[i], e1));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r2[i], m[i] ? e1 : T(0)));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r3[i], m[i] ? e1 : arrSrc[i]));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r4[i], e4));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r5[i], e5));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r6[i], m[i] ? e4 : arrSrc[i]));
                RTTS_EXPECT(rtts::vx_fp::bits_match(r7[i], m[i] ? e5 : arrSrc[i]));
                if (std::isnan(arrA[i])) {
                    RTTS_EXPECT(std::signbit(r1[i]) == std::signbit(arrB[i]));
                }
            }
        };

        for (size_t shift = 0; shift < vals.size(); ++shift) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = vals[(i + shift) % vals.size()];
                arrB[i] = vals[(i * 5 + shift + 3) % vals.size()];
                arrSrc[i] = T(50 + i);
            }
            check(Mask(true));
            check(Mask(false));
            check(rtts::simd::make_alternating_mask<Mask>());
            check(rtts::simd::make_random_mask<Mask>());
        }

        std::mt19937_64 rng(0x24001ULL + sizeof(T) * 29 + N);
        rtts::random::generator<T> gen(0x24002ULL + sizeof(T) * 53 + N);

        for (size_t trial = 0; trial < 300; ++trial) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = (rng() % 4 == 0) ? vals[rng() % vals.size()] : gen();
                arrB[i] = (rng() % 4 == 0) ? vals[rng() % vals.size()] : gen();
                arrSrc[i] = gen();
            }
            check(rtts::simd::make_random_mask<Mask>());
        }
    }
}

RTTS_CASE_TPL("raze::vx::copysign", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    test_copysign_vector<V>();
};

RTTS_CASE("raze::vx::copysign non-native and constraints") {
    test_copysign_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>();
    test_copysign_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>();
    test_copysign_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>();
    test_copysign_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>();
    static_assert(!std::is_invocable_v<decltype(raze::vx::copysign), raze::vx::simd<raze::i32>, raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

