#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

#include <limits>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::convert", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

    auto test_target = [&] <class To> () {
        using DstV = raze::vx::simd<To, Abi>;
        using DstMask = typename DstV::mask_type;

        rtts::random::generator<T> gen(0x12345678ULL + sizeof(T) * 37 + sizeof(To) * 19 + N);
        rtts::random::generator<To> gen_dst(0x87654321ULL + sizeof(To) * 31 + sizeof(T) * 11 + N);

        alignas(alignof(V)) T arrA[N];
        alignas(alignof(DstV)) To arrSrc[N];

        const auto any_domain = rtts::random::domain<T>::standard();
        const auto any_dst_domain = rtts::random::domain<To>::standard();

        for (size_t trial = 0; trial < 1000; ++trial) {
            gen.fill(arrA, any_domain);
            gen_dst.fill(arrSrc, any_dst_domain);

            for (size_t i = 0; i < N; ++i) {
                long long val = 0;
                if constexpr (std::is_floating_point_v<T>) val = static_cast<long long>(trial * 17 + i * 3);
                else val = static_cast<long long>(arrA[i]);

                if constexpr (std::is_unsigned_v<T> || std::is_unsigned_v<To>) arrA[i] = static_cast<T>(std::abs(val) % 100);
                else arrA[i] = static_cast<T>((val % 199) - 99);
            }

            V a = raze::vx::load<V>(arrA);
            DstV src = raze::vx::load<DstV>(arrSrc);
            DstMask m = rtts::simd::make_random_mask<DstMask>();

            auto r1 = raze::vx::convert<To>(a);
            RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return static_cast<To>(arrA[i]); });
        }

        for (size_t i = 0; i < N; ++i) {
            long long val = static_cast<long long>(i * 7 + 5);
            if constexpr (std::is_unsigned_v<T> || std::is_unsigned_v<To>) arrA[i] = static_cast<T>(std::abs(val) % 100);
            else arrA[i] = static_cast<T>((val % 199) - 99);
        }

        V a = raze::vx::load<V>(arrA);
        auto r_roundtrip = raze::vx::convert<T>(raze::vx::convert<To>(a));
        RTTS_ALL_VALIDATE_VECTOR(r_roundtrip, [&](auto i) { return arrA[i]; });
    };

    auto run_all_targets = [&] <class... Targets>() {
        (test_target.template operator()<Targets>(), ...);
    };

    run_all_targets.template operator()<int8_t, uint8_t, int16_t, uint16_t,
        int32_t, uint32_t, int64_t, uint64_t, float, double>();
};

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