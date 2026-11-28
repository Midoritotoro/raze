#include <tests/vx/FloatTestHelper.h>

RAZE_TEST_NAMESPACE_BEGIN

namespace {
    template <class V>
    void test_is_unordered_vector() {
        using T = typename V::value_type;
        using Mask = typename V::mask_type;
        constexpr size_t N = V::size();

        const auto specials = rtts::vx_fp::classify_values<T>();
        alignas(alignof(V)) T arrA[N];
        alignas(alignof(V)) T arrB[N];

        auto check = [&](const Mask& m) {
            const V a = raze::vx::load<V>(arrA);
            const V b = raze::vx::load<V>(arrB);

            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered(a, b), [&](size_t i) {
                return rtts::vx_fp::ref_is_nan(arrA[i]) || rtts::vx_fp::ref_is_nan(arrB[i]);
            });
            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered[m](a, b), [&](size_t i) {
                return m[i] && (rtts::vx_fp::ref_is_nan(arrA[i]) || rtts::vx_fp::ref_is_nan(arrB[i]));
            });
            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered(a, arrB[0]), [&](size_t i) {
                return rtts::vx_fp::ref_is_nan(arrA[i]) || rtts::vx_fp::ref_is_nan(arrB[0]);
            });
            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered(arrA[0], b), [&](size_t i) {
                return rtts::vx_fp::ref_is_nan(arrA[0]) || rtts::vx_fp::ref_is_nan(arrB[i]);
            });
            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered[m](a, arrB[0]), [&](size_t i) {
                return m[i] && (rtts::vx_fp::ref_is_nan(arrA[i]) || rtts::vx_fp::ref_is_nan(arrB[0]));
            });
            RTTS_ALL_VALIDATE_VECTOR(raze::vx::is_unordered[m](arrA[0], b), [&](size_t i) {
                return m[i] && (rtts::vx_fp::ref_is_nan(arrA[0]) || rtts::vx_fp::ref_is_nan(arrB[i]));
            });
        };

        for (size_t shift = 0; shift < specials.size(); ++shift) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = specials[(i + shift) % specials.size()];
                arrB[i] = specials[(i * 3 + shift + 5) % specials.size()];
            }
            check(Mask(true));
            check(Mask(false));
            check(rtts::simd::make_alternating_mask<Mask>());
            check(rtts::simd::make_random_mask<Mask>());
        }

        std::mt19937_64 rng(0x19001ULL + sizeof(T) * 19 + N);
        rtts::random::generator<T> gen(0x19002ULL + sizeof(T) * 31 + N);

        for (size_t trial = 0; trial < 200; ++trial) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = (rng() % 3 == 0) ? specials[rng() % specials.size()] : gen();
                arrB[i] = (rng() % 3 == 0) ? specials[rng() % specials.size()] : gen();
            }
            check(rtts::simd::make_random_mask<Mask>());
        }
    }
}

RTTS_CASE_TPL("raze::vx::is_unordered", rtts::simd::all_fp_infos)
<class Simd>(rtts::type<Simd>) {
    using V = typename Simd::type;
    test_is_unordered_vector<V>();
};

RTTS_CASE("raze::vx::is_unordered non-native and constraints") {
    test_is_unordered_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<3>>>();
    test_is_unordered_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<3>>>();
    test_is_unordered_vector<raze::vx::simd<raze::f32, raze::vx::fixed_abi<16>>>();
    test_is_unordered_vector<raze::vx::simd<raze::f64, raze::vx::fixed_abi<8>>>();
    static_assert(!std::is_invocable_v<decltype(raze::vx::is_unordered), raze::vx::simd<raze::i32>, raze::vx::simd<raze::i32>>);
    RTTS_EXPECT(true);
};

RAZE_TEST_NAMESPACE_END

