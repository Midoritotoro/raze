#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

#include <limits>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::convert trunc_vs_sat in-range", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    auto test_target = [&] <class To> () {
        rtts::random::generator<T> gen(0xBADF00DULL + sizeof(T) * 13 + sizeof(To) * 7 + N);
        alignas(alignof(V)) T arrA[N];
        const auto any_domain = rtts::random::domain<T>::standard();

        for (size_t trial = 0; trial < 1000; ++trial) {
            gen.fill(arrA, any_domain);

            for (size_t i = 0; i < N; ++i) {
                long double val = static_cast<long double>(arrA[i]);
                long double lo = static_cast<long double>(std::numeric_limits<To>::lowest());
                long double hi = static_cast<long double>(std::numeric_limits<To>::max());

                long double t_lo = static_cast<long double>(std::numeric_limits<T>::lowest());
                long double t_hi = static_cast<long double>(std::numeric_limits<T>::max());

                if (lo < t_lo) lo = t_lo;
                if (hi > t_hi) hi = t_hi;

                if (val < lo) val = lo;
                if (val > hi) val = hi;

                if constexpr (std::is_floating_point_v<T>) {
                    T t_val = static_cast<T>(val);
                    if (static_cast<long double>(t_val) > hi) {
                        t_val = std::nextafter(static_cast<T>(hi), static_cast<T>(lo));
                    }
                    if (static_cast<long double>(t_val) < lo) {
                        t_val = std::nextafter(static_cast<T>(lo), static_cast<T>(hi));
                    }
                    arrA[i] = t_val;
                }
                else {
                    arrA[i] = static_cast<T>(val);
                }
            }

            V a = raze::vx::load<V>(arrA);
            auto r_trunc = raze::vx::convert<To>(a);
            auto r_sat = raze::vx::convert<To>[raze::vx::saturated](a);

            RTTS_ALL_VALIDATE_VECTOR(r_trunc, [&](auto i) { return r_sat[i]; });
        }
    };

    auto run_all_targets = [&] <class... Targets>() {
        (test_target.template operator()<Targets>(), ...);
    };

    run_all_targets.template operator()<int8_t, uint8_t, int16_t, uint16_t,
        int32_t, uint32_t, int64_t, uint64_t, float, double>();
};

RAZE_TEST_NAMESPACE_END