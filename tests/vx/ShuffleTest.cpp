#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>
#include <random>

template <
    class           Simd,
    raze::u64    Seed>
void test_shuffle_with_compile_time_pattern() {
    using T = typename Simd::value_type;
    constexpr std::size_t N = Simd::size();

    alignas(64) T arr[N];
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<T>(i + 1);

    Simd v = raze::vx::load<Simd>(arr);

    {
        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::vx::shuffle(v, raze::vx::make_pattern<Simd, static_cast<raze::u64>((I * Seed * 1103515245u + 12345u) % N)...>{});

            constexpr std::size_t indices[] = { I... };
            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[(indices[i] * Seed * 1103515245u + 12345u) % N]);
        }(std::make_index_sequence<N>{});
    }

    {
        constexpr auto make_idx = [](std::size_t i) constexpr {
            return (Seed * 6364136223846793005ULL + i * 1442695040888963407ULL) % N;
        };

        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::vx::shuffle(v, raze::vx::make_pattern<Simd, static_cast<raze::u64>(make_idx(I))...>{});

            constexpr std::size_t idxs[] = { make_idx(I)... };

            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[idxs[i]]);
        }(std::make_index_sequence<N>{});
    }

    {
        constexpr std::size_t a = (Seed * 1664525u + 1013904223u) % N;
        constexpr std::size_t b = (Seed * 22695477u + 1u) % N;

        constexpr auto make_idx = [](std::size_t i) constexpr {
            return (a * i + b) % N;
        };

        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::vx::shuffle(v, raze::vx::make_pattern<Simd, static_cast<raze::u64>(make_idx(I))...>{});

            constexpr std::size_t idxs[] = { make_idx(I)... };

            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[idxs[i]]);
        }(std::make_index_sequence<N>{});
    }
}

template <
    class               Simd,
    raze::u64 ...    Seeds>
void test_shuffle_with_compile_time_all_patterns(std::index_sequence<Seeds...>) {
    (test_shuffle_with_compile_time_pattern<Simd, Seeds>(), ...);
}

template <class Simd>
void test_shuffle_runtime_pattern()
{
    using T = typename Simd::value_type;
    using U = typename raze::IntegerForSizeof<T>::Unsigned;

    using IndexSimd =
        raze::vx::simd<U, typename Simd::abi_type>;

    constexpr std::size_t N = Simd::size();

    alignas(64) T src[N];
    alignas(64) U idx[N];

    for (std::size_t i = 0; i < N; ++i)
        src[i] = static_cast<T>(i + 1);

    Simd v = raze::vx::load<Simd>(src);

    for (std::size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(i);

    {
        auto r = raze::vx::shuffle(
            v,
            raze::vx::load<IndexSimd>(idx));

        for (std::size_t i = 0; i < N; ++i)
            raze_assert(r[i] == src[i]);
    }

    for (std::size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(N - 1 - i);

    {
        auto r = raze::vx::shuffle(
            v,
            raze::vx::load<IndexSimd>(idx));

        for (std::size_t i = 0; i < N; ++i)
            raze_assert(r[i] == src[idx[i]]);
    }

    for (std::size_t i = 0; i < N; ++i)
        idx[i] = 0;

    {
        auto r = raze::vx::shuffle(
            v,
            raze::vx::load<IndexSimd>(idx));

        for (std::size_t i = 0; i < N; ++i)
            raze_assert(r[i] == src[0]);
    }

    for (std::size_t i = 0; i < N; ++i)
        idx[i] = static_cast<U>(N - 1);

    {
        auto r = raze::vx::shuffle(
            v,
            raze::vx::load<IndexSimd>(idx));

        for (std::size_t i = 0; i < N; ++i)
            raze_assert(r[i] == src[N - 1]);
    }
}

template <
    class Simd,
    raze::u64 Seed,
    std::size_t Iterations>
void test_shuffle_runtime_random()
{
    using T = typename Simd::value_type;
    using U = typename raze::IntegerForSizeof<T>::Unsigned;

    using IndexSimd = raze::vx::simd<U, typename Simd::abi_type>;
    constexpr std::size_t N = Simd::size();

    std::mt19937_64 rng(Seed);

    alignas(64) T src[N];
    alignas(64) T ref[N];
    alignas(64) U idx[N];

    for (std::size_t it = 0; it < Iterations; ++it)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if constexpr (std::is_integral_v<T>)
                src[i] = static_cast<T>(rng());
            else
                src[i] = static_cast<T>(
                    static_cast<std::int64_t>(rng()));

            idx[i] = static_cast<U>(rng() % N);
        }

        for (std::size_t i = 0; i < N; ++i)
            ref[i] = src[idx[i]];

        Simd v = raze::vx::load<Simd>(src);
        IndexSimd vi = raze::vx::load<IndexSimd>(idx);

        auto r = raze::vx::shuffle(v, vi);

        for (std::size_t i = 0; i < N; ++i)
            raze_assert(r[i] == ref[i]);
    }
}

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct shuffle_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        //test_shuffle_runtime_pattern<Simd>();
        //test_shuffle_runtime_random<Simd, 0x12345678ULL, 30000>();
        //test_shuffle_runtime_random<Simd, 0xCAFEBABEULL, 30000>();
        //test_shuffle_runtime_random<Simd,  0xDEADBEEF12345678ULL, 30000>();

        test_shuffle_with_compile_time_all_patterns<Simd>(std::make_index_sequence<128>{});
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<(_Width_ / (sizeof(_Type_) * 8)) * 2>();
    }
};

int main() {
    test_all<shuffle_tests>();
    return 0;
}
