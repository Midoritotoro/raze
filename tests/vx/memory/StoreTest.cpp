#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::store", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();
    constexpr size_t PAD = 4;

    T fallback[N];
    for (size_t i = 0; i < N; ++i)
        fallback[i] = static_cast<T>(42 + i);

    V fallback_v = raze::vx::load<V>(fallback);
    V val;
    
    {
        alignas(std::hardware_constructive_interference_size) T tmp[N];

        for (size_t i = 0; i < N; ++i)
            tmp[i] = static_cast<T>(i * 13 + 5);

        val = raze::vx::load<V>(tmp);
    }

    for (size_t iteration = 0; iteration < 100; ++iteration) {
        (void)iteration;

        Mask mask = rtts::simd::make_random_mask<Mask>();

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];
            raze::vx::store[raze::vx::aligned](dst, val);

            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(dst[i] == val[i]);
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* dst = buf + 1;

            raze::vx::store(dst, val);

            for (size_t i = 0; i < N; ++i)
                RTTS_EXPECT(dst[i] == val[i]);
        }

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0xDEADBEEF);

            raze::vx::store[mask](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : static_cast<T>(0xDEADBEEF);
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0xCAFEBABE);

            raze::vx::store[mask][raze::vx::aligned](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : static_cast<T>(0xCAFEBABE);
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* dst = buf + 1;

            raze::vx::store[mask, fallback_v](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : fallback_v[i];
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];

            raze::vx::store[mask, fallback_v][raze::vx::aligned](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : fallback_v[i];
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0xDEADBEEF);

            raze::vx::store[mask][raze::vx::safe](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : static_cast<T>(0xDEADBEEF);
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* dst = buf + 1;

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0xCAFEBABE);

            raze::vx::store[mask][raze::vx::safe](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : static_cast<T>(0xCAFEBABE);
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T dst[N];

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0x12345678);

            raze::vx::store[mask, fallback_v][raze::vx::safe](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : fallback_v[i];
                RTTS_EXPECT(dst[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* dst = buf + 1;

            for (size_t i = 0; i < N; ++i)
                dst[i] = static_cast<T>(0x87654321);

            raze::vx::store[mask, fallback_v][raze::vx::safe](dst, val);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? val[i] : fallback_v[i];
                RTTS_EXPECT(dst[i] == expected);
            }
        }
    }
};

RAZE_TEST_NAMESPACE_END
