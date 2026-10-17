#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

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

    run_all_targets.template operator()<int8_t, uint8_t>();
};

RAZE_TEST_NAMESPACE_END