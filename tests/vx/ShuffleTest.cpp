#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>


template <
    class           Simd,
    raze::u64    Seed>
void test_shuffle_with_compile_time_pattern() {
    using T = typename Simd::value_type;
    constexpr std::size_t N = Simd::size();

    alignas(64) T arr[N];
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<T>(i + 1);

    Simd v; v.copy_from(arr);

    {
        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::vx::shuffle(v, std::integer_sequence<raze::u64, static_cast<raze::u64>((I * Seed * 1103515245u + 12345u) % N)...>{});

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
            auto r = raze::vx::shuffle(v, std::integer_sequence<raze::u64, static_cast<raze::u64>(make_idx(I))...>{});

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
            auto r = raze::vx::shuffle(v, std::integer_sequence<raze::u64, static_cast<raze::u64>(make_idx(I))...>{});

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

        

        // test_shuffle_with_compile_time_all_patterns<Simd>(std::make_index_sequence<4>{});
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
    }
};

int main() {
    test_all<shuffle_tests>();
    return 0;
}
