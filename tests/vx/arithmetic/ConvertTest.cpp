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

        for (size_t trial = 0; trial < 100; ++trial) {
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

//RTTS_CASE_TPL("raze::vx::convert[saturated]", rtts::simd::all_simd_infos)
//<class Simd> (rtts::type<Simd>) {
//    using V = typename Simd::type;
//    using T = typename V::value_type;
//    using Abi = typename V::abi_type;
//    constexpr size_t N = V::size();
//
//    auto test_target = [&] <class To> () {
//        rtts::random::generator<T> gen(0xCAFEBABEULL + sizeof(T) * 37 + sizeof(To) * 19 + N);
//        alignas(alignof(V)) T arrA[N];
//        const auto any_domain = rtts::random::domain<T>::standard();
//
//        constexpr bool float_to_wide_signed = std::is_floating_point_v<T> &&
//            !std::is_floating_point_v<To> && std::is_signed_v<To> && sizeof(To) >= 4;
//
//        auto reference = [](T v) -> To {
//            if constexpr (std::is_floating_point_v<To>) {
//                return static_cast<To>(v);
//            } else if constexpr (float_to_wide_signed) {
//                return static_cast<To>(v);
//            } else {
//                long double d  = static_cast<long double>(v);
//                long double lo = static_cast<long double>(std::numeric_limits<To>::lowest());
//                long double hi = static_cast<long double>(std::numeric_limits<To>::max());
//                if (!(d == d)) return To{};
//                d = d < lo ? lo : (d > hi ? hi : d);
//                return static_cast<To>(d);
//            }
//        };
//
//        for (size_t trial = 0; trial < 100; ++trial) {
//            gen.fill(arrA, any_domain);
//
//            for (size_t i = 0; i < N; ++i) {
//                long double lo = static_cast<long double>(std::numeric_limits<To>::lowest());
//                long double hi = static_cast<long double>(std::numeric_limits<To>::max());
//                long double v;
//                if constexpr (float_to_wide_signed) {
//                    v = ((long double)((trial * 131 + i * 17) % 20000) - 10000.0L) * 100.0L;
//                } else {
//                    switch ((trial * 7 + i * 3) % 8) {
//                        case 0:  v = lo * 0.5L; break;
//                        case 1:  v = lo;        break;
//                        case 2:  v = lo - 1;    break;
//                        case 3:  v = lo * 10;   break;
//                        case 4:  v = hi * 0.5L; break;
//                        case 5:  v = hi;        break;
//                        case 6:  v = hi + 1;    break;
//                        default: v = hi * 10;   break;
//                    }
//                }
//
//                if constexpr (std::is_floating_point_v<T>) {
//                    arrA[i] = static_cast<T>(v);
//                } else {
//                    long double t_lo = static_cast<long double>(std::numeric_limits<T>::lowest());
//                    long double t_hi = static_cast<long double>(std::numeric_limits<T>::max());
//                    if (v < t_lo) v = t_lo;
//                    if (v > t_hi) v = t_hi;
//                    arrA[i] = static_cast<T>(v);
//                }
//            }
//
//            V a = raze::vx::load<V>(arrA);
//            auto r = raze::vx::convert<To>[raze::vx::saturated](a);
//            RTTS_ALL_VALIDATE_VECTOR(r, [&](auto i) { return reference(arrA[i]); });
//        }
//    };
//
//    auto run_all_targets = [&] <class... Targets>() {
//        (test_target.template operator()<Targets>(), ...);
//    };
//
//    run_all_targets.template operator()<int8_t, uint8_t, int16_t, uint16_t,
//        int32_t, uint32_t, int64_t, uint64_t, float, double>();
//};
//
//RTTS_CASE_TPL("raze::vx::convert trunc_vs_sat in-range", rtts::simd::all_simd_infos)
//<class Simd> (rtts::type<Simd>) {
//    using V = typename Simd::type;
//    using T = typename V::value_type;
//    using Abi = typename V::abi_type;
//    constexpr size_t N = V::size();
//
//    auto test_target = [&] <class To> () {
//        rtts::random::generator<T> gen(0xBADF00DULL + sizeof(T) * 13 + sizeof(To) * 7 + N);
//        alignas(alignof(V)) T arrA[N];
//        const auto any_domain = rtts::random::domain<T>::standard();
//
//        for (size_t trial = 0; trial < 50; ++trial) {
//            gen.fill(arrA, any_domain);
//
//            for (size_t i = 0; i < N; ++i) {
//                long double lo = static_cast<long double>(std::numeric_limits<To>::lowest()) * 0.9L;
//                long double hi = static_cast<long double>(std::numeric_limits<To>::max())    * 0.9L;
//                long double d  = static_cast<long double>(arrA[i]);
//                d = d < lo ? lo : (d > hi ? hi : d);
//
//                if constexpr (std::is_floating_point_v<T>) {
//                    arrA[i] = static_cast<T>(d);
//                } else {
//                    long double t_lo = static_cast<long double>(std::numeric_limits<T>::lowest());
//                    long double t_hi = static_cast<long double>(std::numeric_limits<T>::max());
//                    if (d < t_lo) d = t_lo;
//                    if (d > t_hi) d = t_hi;
//                    arrA[i] = static_cast<T>(d);
//                }
//            }
//
//            V a = raze::vx::load<V>(arrA);
//            auto r_trunc = raze::vx::convert<To>(a);
//            auto r_sat   = raze::vx::convert<To>[raze::vx::saturated](a);
//
//            RTTS_ALL_VALIDATE_VECTOR(r_trunc, [&](auto i) { return r_sat[i]; });
//        }
//    };
//
//    auto run_all_targets = [&] <class... Targets>() {
//        (test_target.template operator()<Targets>(), ...);
//    };
//
//    run_all_targets.template operator()<int8_t, uint8_t, int16_t, uint16_t,
//        int32_t, uint32_t, int64_t, uint64_t, float, double>();
//};

RAZE_TEST_NAMESPACE_END