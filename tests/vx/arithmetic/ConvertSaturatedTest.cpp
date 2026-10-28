#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

#include <limits>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::convert[saturated]", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    auto test_target = [&] <class To> () {
        rtts::random::generator<T> gen(0xCAFEBABEULL + sizeof(T) * 37 + sizeof(To) * 19 + N);
        alignas(alignof(V)) T arrA[N];
        const auto any_domain = rtts::random::domain<T>::standard();

        for (size_t trial = 0; trial < 1000; ++trial) {
            if (trial < 50) {
                gen.fill(arrA, any_domain);
            } 
            else {
                long double lo = static_cast<long double>(std::numeric_limits<To>::lowest());
                long double hi = static_cast<long double>(std::numeric_limits<To>::max());

                for (size_t i = 0; i < N; ++i) {
                    long double v;
                    switch ((trial * 7 + i * 3) % 10) {
                        case 0:  v = lo * 0.5L; break;
                        case 1:  v = lo;        break;
                        case 2:  v = lo - 1.0L; break;
                        case 3:  v = (lo == 0.0L) ? -10.0L : lo * 10.0L; break;
                        case 4:  v = hi * 0.5L; break;
                        case 5:  v = hi;        break;
                        case 6:  v = hi + 1.0L; break;
                        case 7:  v = hi * 10.0L; break;
                        case 8:  v = 0.0L;      break;
                        default: v = -1.0L;     break;
                    }

                    if constexpr (std::is_floating_point_v<T>) {
                        constexpr long double t_lo = static_cast<long double>(std::numeric_limits<T>::lowest());
                        constexpr long double t_hi = static_cast<long double>(std::numeric_limits<T>::max());
                        if (v < t_lo) v = t_lo;
                        if (v > t_hi) v = t_hi;
                        arrA[i] = static_cast<T>(v);
                    } 
                    else {
                        constexpr auto t_lo = std::numeric_limits<T>::lowest();
                        constexpr auto t_hi = std::numeric_limits<T>::max();
                        if (v <= static_cast<long double>(t_lo)) {
                            arrA[i] = t_lo;
                        } else if (v >= static_cast<long double>(t_hi)) {
                            arrA[i] = t_hi;
                        } else {
                            arrA[i] = static_cast<T>(v);
                        }
                    }
                }
            }

            V a = raze::vx::load<V>(arrA);
            auto r = raze::vx::convert<To>[raze::vx::saturated](a);
            RTTS_ALL_VALIDATE_VECTOR(r, [&](auto i) { return raze::math::saturating_cast<To>(arrA[i]); });
        }
    };

    auto run_all_targets = [&] <class... Targets>() {
        (test_target.template operator()<Targets>(), ...);
    };

    run_all_targets.template operator()<int8_t, uint8_t, int16_t, uint16_t,
        int32_t, uint32_t, int64_t, uint64_t, float, double>();
};
 
RAZE_TEST_NAMESPACE_END