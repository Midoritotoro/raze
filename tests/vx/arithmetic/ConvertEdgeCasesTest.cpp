#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

#include <limits>

RAZE_TEST_NAMESPACE_BEGIN
 
RTTS_CASE_TPL("raze::vx::convert manual edge cases", rtts::simd::all_simd_infos)
< class Simd > (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    auto test_target = [&] <class To> () {
        alignas(alignof(V)) T arrA[N];
        std::vector<long double> edge_cases;

        edge_cases.push_back(0.0L);
        edge_cases.push_back(1.0L);
        edge_cases.push_back(-1.0L);
        edge_cases.push_back(0.5L);
        edge_cases.push_back(-0.5L);

        long double to_min = static_cast<long double>(std::numeric_limits<To>::lowest());
        long double to_max = static_cast<long double>(std::numeric_limits<To>::max());

        edge_cases.push_back(to_min);
        edge_cases.push_back(to_max);
        edge_cases.push_back(to_min - 0.5L); 
        edge_cases.push_back(to_max + 0.5L);
        edge_cases.push_back(to_min - 10.0L);
        edge_cases.push_back(to_max + 10.0L);

        if constexpr (std::is_floating_point_v<T> && std::is_integral_v<To>) {
            edge_cases.push_back(static_cast<long double>(to_max) + 0.99L);
            edge_cases.push_back(static_cast<long double>(to_min) - 0.99L);
        }

        for (size_t i = 0; i < N; ++i) {
            long double val = edge_cases[i % edge_cases.size()];

            if constexpr (std::is_floating_point_v<T>) {
                arrA[i] = static_cast<T>(val);
            }
            else {
                if (val < static_cast<long double>(std::numeric_limits<T>::lowest()))
                    arrA[i] = std::numeric_limits<T>::lowest();
                else if (val > static_cast<long double>(std::numeric_limits<T>::max()))
                    arrA[i] = std::numeric_limits<T>::max();
                else
                    arrA[i] = static_cast<T>(val);
            }
        }

        V a = raze::vx::load<V>(arrA);
        auto r_trunc = raze::vx::convert<To>(a);
        auto r_sat = raze::vx::convert<To>[raze::vx::saturated](a);

        for (size_t i = 0; i < N; ++i) {
            T src_val = arrA[i];
            long double exact_src = static_cast<long double>(src_val);
            if (exact_src >= static_cast<long double>(std::numeric_limits<To>::lowest()) &&
                exact_src <= static_cast<long double>(std::numeric_limits<To>::max()))
            {
                To expected_trunc = static_cast<To>(src_val);
                RTTS_EXPECT(r_trunc[i] == expected_trunc);
            }

            To expected_sat = raze::math::saturating_cast<To>(src_val);
            RTTS_EXPECT(r_sat[i] == expected_sat);
        }
    };

    auto run_all_targets = [&] <class... Targets>() {
        (test_target.template operator() < Targets > (), ...);
    };

    run_all_targets.template operator() < int8_t, uint8_t, int16_t, uint16_t,
        int32_t, uint32_t, int64_t, uint64_t, float, double > ();
};
 
RAZE_TEST_NAMESPACE_END