#include <tests/rts/rts.h>
#include <raze/vx/Simd.h>
#include <raze/math/Math.h>

#include <cmath>
#include <limits>
#include <random>
#include <type_traits>

RAZE_TEST_NAMESPACE_BEGIN

template<class T>
void test_scalar_abs() {
    auto check_result = [] (T r, T expected) {
        if constexpr (std::is_floating_point_v<T>) {
            if (std::isnan(expected)) RTTS_EXPECT(std::isnan(r));
            else RTTS_EXPECT(r == expected);
        } 
        else { RTTS_EXPECT(r == expected); }
    };

    auto test_value = [&] (T x, T expected) { check_result(raze::math::abs(x), expected); };
    auto test_cond = [&] (T x, T expected, bool cond) { check_result(raze::math::abs[cond](x), cond ? expected : T(0)); };
    auto test_cond_fbk = [&] (T x, bool cond, T fallback) { check_result(raze::math::abs[cond, fallback](x), cond ? std::abs(x) : fallback); };

    std::mt19937_64 rng(0x123456789ABCDEFULL);

    if constexpr (std::is_signed_v<T>) {
        for (size_t i = 0; i < 1000; ++i)
            test_value(static_cast<T>(rng()), std::abs(static_cast<T>(rng())));

        for (size_t i = 0; i < 1000; ++i) {
            const T x = static_cast<T>(rng());
            test_cond(x, std::abs(x), (i & 1) == 0);
        }

        for (size_t i = 0; i < 1000; ++i)
            test_cond_fbk(static_cast<T>(rng()), (i & 1) == 0, static_cast<T>(rng()));

        test_value(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

        const T min_val = std::numeric_limits<T>::min();
        test_value(min_val, (min_val < 0) ? T(-(min_val + 1)) + 1 : min_val);
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

    alignas(64) T arr[N], fallback[N];
    std::mt19937_64 rng(0x123456789ABCDEFULL);

    for (size_t i = 0; i < N; ++i) {
        fallback[i] = T(i + 1);
        arr[i] = (std::is_signed_v<T> && (i & 1)) ? T(i + 1) : T(-(T(i) + 1));
    }

    V v = raze::vx::load<V>(arr);
    V fbk = raze::vx::load<V>(fallback);

    auto check_abs = [&] (V r) {
        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == (arr[i] < T(0) ? T(-arr[i]) : arr[i]));
    };

    check_abs(raze::math::abs(v));

    auto check_mask = [&] (Mask m) {
        check_abs(raze::math::abs[m](v));
        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(raze::math::abs[m, fbk](v)[i] == (m[i] ? raze::math::abs(arr[i]) : fallback[i]));
    };

    constexpr size_t ExhaustiveMasks = (N < 7) ? (size_t(1) << N) : size_t(0);

    if constexpr (ExhaustiveMasks != 0) {
        for (size_t bits = 0; bits < ExhaustiveMasks; ++bits) {
            Mask m;
            for (size_t i = 0; i < N; ++i)
                m[i] = ((bits >> i) & 1) != 0;
            check_mask(m);
        }
    } else {
        std::array<std::pair<Mask, const char*>, 4> masks;
        for (auto& [m, _] : masks)
            for (size_t i = 0; i < N; ++i) {
                m[i] = (&m == &masks[0].first) ? false
                     : (&m == &masks[1].first) ? true
                     : (&m == &masks[2].first) ? (i & 1) == 0
                     : (i & 1) != 0;
            }

        for (auto& [m, _] : masks)
            check_mask(m);

        for (size_t iteration = 0; iteration < 100; ++iteration) {
            Mask m;
            for (size_t i = 0; i < N; ++i)
                m[i] = (rng() & 1) != 0;
            check_mask(m);
        }
    }
};

RAZE_TEST_NAMESPACE_END
