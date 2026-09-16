#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <numeric>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::load", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();
    constexpr size_t PAD = 4;

    T fallback[N];
    std::iota(fallback, fallback + N, T(42));

    V fallback_v = raze::vx::load<V>(fallback);

    {
        alignas(std::hardware_constructive_interference_size) T src[N];

        for (size_t i = 0; i < N; ++i)
            src[i] = static_cast<T>(i * 3 + 1);

        V v = raze::vx::load<V>[raze::vx::aligned](src);
        RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return src[i]; });
    }

    {
        alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
        T* src = buf + 1;

        for (size_t i = 0; i < N; ++i)
            src[i] = static_cast<T>(i * 7 + 2);

        V v = raze::vx::load<V>(src);

        RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return src[i]; });
    }

    for (size_t iteration = 0; iteration < 100; ++iteration) {
        (void)iteration;
        Mask mask = rtts::simd::make_random_mask<Mask>();

        {
            alignas(std::hardware_constructive_interference_size) T src[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 3 + 1);

            V v = raze::vx::load<V>[mask](src);
            RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return mask[i] ? src[i] : T(0); });
        }

        {
            alignas(std::hardware_constructive_interference_size) T src[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 5 + 2);

            V v = raze::vx::load<V>[mask][raze::vx::aligned](src);
            RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return mask[i] ? src[i] : T(0); });
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* src = buf + 1;

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 7 + 3);

            V v = raze::vx::load<V>[mask, fallback_v](src);
            RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return mask[i] ? src[i] : fallback_v[i]; });
        }

        {
            alignas(std::hardware_constructive_interference_size) T src[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 11 + 4);

            V v = raze::vx::load<V>[mask, fallback_v][raze::vx::aligned](src);
            RTTS_ALL_VALIDATE_BITS(v, [&](auto i) { return mask[i] ? src[i] : fallback_v[i]; });
        }

        {
            alignas(std::hardware_constructive_interference_size) T src[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 3 + 1);

            V v = raze::vx::load<V>[mask][raze::vx::safe](src);

            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) RTTS_EXPECT(v[i] == src[i]);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* src = buf + 1;

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 5 + 3);

            V v = raze::vx::load<V>[mask][raze::vx::safe](src);

            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) RTTS_EXPECT(v[i] == src[i]);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T src[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 7 + 2);

            V v = raze::vx::load<V>[mask, fallback_v][raze::vx::safe](src);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? src[i] : fallback_v[i];
                RTTS_EXPECT(v[i] == expected);
            }
        }

        {
            alignas(std::hardware_constructive_interference_size) T buf[N * 2 + PAD];
            T* src = buf + 1;

            for (size_t i = 0; i < N; ++i)
                src[i] = static_cast<T>(i * 11 + 4);

            V v = raze::vx::load<V>[mask, fallback_v][raze::vx::safe](src);

            for (size_t i = 0; i < N; ++i) {
                T expected = mask[i] ? src[i] : fallback_v[i];
                RTTS_EXPECT(v[i] == expected);
            }
        }
    }
};

RAZE_TEST_NAMESPACE_END
