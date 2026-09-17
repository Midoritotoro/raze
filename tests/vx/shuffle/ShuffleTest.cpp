#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

#include <random>
#include <type_traits>

RAZE_TEST_NAMESPACE_BEGIN

template <class V, raze::u64 Seed, size_t... I>
constexpr auto make_shuffle_pattern_lcg(std::index_sequence<I...>) {
    constexpr size_t N = V::size();
    return raze::vx::make_pattern<V, static_cast<raze::u64>((I * Seed * 1103515245u + 12345u) % N)...>{};
}

template <class V, size_t Seed>
void test_shuffle_compile_time_pattern(const V& v, const auto& src) {
    constexpr size_t N = V::size();
    auto r = raze::vx::shuffle(v, make_shuffle_pattern_lcg<V, Seed>(std::make_index_sequence<N>{}));

    for (size_t i = 0; i < N; ++i)
        RTTS_EXPECT(r[i] == src[(i * Seed * 1103515245u + 12345u) % N]);
}

RTTS_CASE_TPL("raze::vx::shuffle", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using U = typename raze::IntegerForSizeof<T>::Unsigned;

    using IndexSimd = raze::vx::simd<U, typename V::abi_type>;

    constexpr size_t N = V::size();

    T src[N];
    U idx[N];

    for (size_t i = 0; i < N; ++i)
        src[i] = T(i + 1);

    V v = raze::vx::load<V>(src);

    test_shuffle_compile_time_pattern<V, 0>(v, src);
    test_shuffle_compile_time_pattern<V, 1>(v, src);
    test_shuffle_compile_time_pattern<V, 2>(v, src);
    test_shuffle_compile_time_pattern<V, 3>(v, src);
    test_shuffle_compile_time_pattern<V, 7>(v, src);
    test_shuffle_compile_time_pattern<V, 13>(v, src);

    for (size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(i);

    {
        auto r = raze::vx::shuffle(v, raze::vx::load<IndexSimd>(idx));

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == src[i]);
    }

    for (size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(N - 1 - i);

    {
        auto r = raze::vx::shuffle(v, raze::vx::load<IndexSimd>(idx));

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == src[N - 1 - i]);
    }

    for (size_t i = 0; i < N; ++i)
        idx[i] = 0;

    {
        auto r = raze::vx::shuffle(v, raze::vx::load<IndexSimd>(idx));

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == src[0]);
    }

    for (size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(N - 1);

    {
        auto r = raze::vx::shuffle(v, raze::vx::load<IndexSimd>(idx));

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == src[N - 1]);
    }

    std::mt19937_64 rng(0xDEADBEEF12345678ULL);

    T random_src[N];
    T expected[N];

    for (size_t it = 0; it < 1024; ++it) {
        for (size_t i = 0; i < N; ++i) {
            if constexpr (std::is_integral_v<T>)
                random_src[i] = static_cast<T>(rng());
            else
                random_src[i] = static_cast<T>(static_cast<std::int64_t>(rng()));

            idx[i] = static_cast<U>(rng() % N);
        }

        for (size_t i = 0; i < N; ++i) 
            expected[i] = random_src[idx[i]];

        V random_v = raze::vx::load<V>(random_src);
        IndexSimd random_idx = raze::vx::load<IndexSimd>(idx);

        auto r = raze::vx::shuffle(random_v, random_idx);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(r[i] == expected[i]);
    }
};

RAZE_TEST_NAMESPACE_END