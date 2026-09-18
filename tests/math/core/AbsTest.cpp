#include <tests/rts/rts.h>
#include <raze/vx/Simd.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

template <class T>
void test_scalar_abs() {
    auto check_result = [] (T r, T expected) {
        if constexpr (std::is_floating_point_v<T>) {
            if (std::isnan(expected)) RTTS_EXPECT(std::isnan(r));
            else RTTS_EXPECT(r == expected);
        } 
        else { RTTS_EXPECT(r == expected); }
    };

    auto test_value = [check_result] (auto x, auto expected) { check_result(raze::math::abs(x), expected); };
    auto test_cond = [check_result] (auto x, auto expected, bool cond) { check_result(raze::math::abs[cond](x), cond ? expected : T(0)); };
    auto test_cond_fbk = [check_result] (auto x, bool cond, auto fallback) { check_result(raze::math::abs[cond, fallback](x), cond ? std::abs(x) : fallback); };

    std::mt19937_64 rng(0x123456789ABCDEFULL);

    if constexpr (std::is_signed_v<T>) {
        for (size_t i = 0; i < 50; ++i) {
            const T x = static_cast<T>(rng());
            test_value(x, std::abs(x));
        }

        for (size_t i = 0; i < 50; ++i) {
            const T x = static_cast<T>(rng());
            test_cond(x, std::abs(x), (i & 1) == 0);
        }

        for (size_t i = 0; i < 50; ++i) {
            const T x = static_cast<T>(rng());
            const T fallback = static_cast<T>(rng());
            test_cond_fbk(x, (i & 1) == 0, fallback);
        }

        test_value(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
    } 
    else {
        test_value(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
    }
}

RTTS_CASE_TPL("raze::math::abs", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();

    test_scalar_abs<T>();

    T arr[N];
    T fallback[N];

    for (size_t i = 0; i < N; ++i) {
        fallback[i] = T(100 + i);
        if constexpr (std::is_signed_v<T>) arr[i] = (i & 1) ? T(i + 1) : T(-(T(i) + 1));
        else arr[i] = T(i + 1);
    }

    V v = raze::vx::load<V>(arr);
    V fbk = raze::vx::load<V>(fallback);

    auto check_abs = [&] (V r) {
        for (size_t i = 0; i < N; ++i) {
            const T expected = arr[i] < T(0) ? T(-arr[i]) : arr[i];
            RTTS_EXPECT(r[i] == expected);
        }
    };

    check_abs(raze::math::abs(v));

    auto check_mask = [&] (Mask m) {
        {
            auto r = raze::math::abs[m](v);

            for (size_t i = 0; i < N; ++i) {
                const T expected_m = m[i] ? raze::math::abs(arr[i]) : T(0);
                if (r[i] != expected_m) {
                    int d = 0;
                }
                RTTS_EXPECT(r[i] == expected_m);
            }
        }

        {
            auto r = raze::math::abs[m, fbk](v);

            for (size_t i = 0; i < N; ++i) {
                const T expected_mfbk = m[i] ? raze::math::abs(arr[i]) : fallback[i];
                RTTS_EXPECT(r[i] == expected_mfbk);
            }
        }
    };

    for (size_t i = 0; i < 100; ++i)
        check_mask(rtts::simd::make_random_mask<Mask>());
};

RAZE_TEST_NAMESPACE_END
