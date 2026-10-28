#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

#include <limits>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::convert special floats", rtts::simd::all_simd_infos)
< class Simd > (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    if constexpr (std::is_floating_point_v<T>) {
        auto test_target = [&] <class To> () {
            alignas(alignof(V)) T arrA[N];

            std::vector<T> special_vals = {
                std::numeric_limits<T>::quiet_NaN(),
                std::numeric_limits<T>::infinity(),
                -std::numeric_limits<T>::infinity(),
                static_cast<T>(-0.0),    
                std::numeric_limits<T>::denorm_min(), 
                -std::numeric_limits<T>::denorm_min() 
            };

            for (size_t i = 0; i < N; ++i) {
                arrA[i] = special_vals[i % special_vals.size()];
            }

            V a = raze::vx::load<V>(arrA);
            auto r_sat = raze::vx::convert<To>[raze::vx::saturated](a);

            for (size_t i = 0; i < N; ++i) {
                To expected_sat = raze::math::saturating_cast<To>(arrA[i]);
                RTTS_EXPECT(r_sat[i] == expected_sat);
            }
        };

        auto run_all_targets = [&] <class... Targets>() {
            (test_target.template operator() < Targets > (), ...);
        };

        run_all_targets.template operator() < int8_t, uint8_t, int16_t, uint16_t,
            int32_t, uint32_t, int64_t, uint64_t > ();
    }
};


RTTS_CASE_TPL("raze::vx::convert float rounding", rtts::simd::all_simd_infos)
< class Simd > (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    if constexpr (std::is_floating_point_v<T>) {
        auto test_target = [&] <class To> () {
            if constexpr (std::is_integral_v<To>) {
                alignas(alignof(V)) T arrA[N];
                T vals[] = { 0.5f, 1.5f, 2.5f, -0.5f, -1.5f, -2.5f, 0.99f, -0.99f };

                for (size_t i = 0; i < N; ++i) arrA[i] = vals[i % 8];

                V a = raze::vx::load<V>(arrA);
                auto r = raze::vx::convert<To>(a);

                for (size_t i = 0; i < N; ++i) {
                    To expected = static_cast<To>(arrA[i]);
                    RTTS_EXPECT(r[i] == expected);
                }
            }
        };
        auto run_all_targets = [&] <class... Targets>() {
            (test_target.template operator() < Targets > (), ...);
        };
        run_all_targets.template operator() < int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t > ();
    }
};

RAZE_TEST_NAMESPACE_END