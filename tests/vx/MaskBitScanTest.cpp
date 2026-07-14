#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>
#include <random>
#include <algorithm>
#include <iostream>

template <class _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct variable_length_mask_tests {

    template <raze::u32 _N_>
    void test_size() const {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        using Mask = raze::vx::simd_mask<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;

        constexpr size_t N = _N_;

        if (!raze::arch::ProcessorFeatures::isSupported<Simd::__isa>())
            return;

        auto make_pattern = [](auto f) {
            Mask m(false);
            for (size_t i = 0; i < N; ++i)
                m[i] = f(i);
            return m;
        };

        auto expected_find_next = [](const Mask& m, size_t pos) {
            for (size_t i = pos + 1; i < N; ++i)
                if (m[i])
                    return i;
            return N;
        };

        {
            Mask m(false);

            raze_assert(raze::vx::count_set(m) == 0);
            raze_assert(raze::vx::find_first_set(m) == N);
            raze_assert(raze::vx::find_last_set(m) == N);
        }

        {
            Mask m(true);

            raze_assert(raze::vx::count_set(m) == N);
            raze_assert(raze::vx::find_first_set(m) == 0);
            raze_assert(raze::vx::find_last_set(m) == 0);
        }

        for (size_t i = 0; i < N; ++i) {
            Mask m(false);
            m[i] = true;

            raze_assert(raze::vx::count_set(m) == 1);
            raze_assert(raze::vx::find_first_set(m) == i);
            raze_assert(raze::vx::find_last_set(m) == (N - i - 1));
        }

        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });

            size_t expected = (N + 1) / 2;

            raze_assert(raze::vx::count_set(m) == expected);
            raze_assert(raze::vx::find_first_set(m) == 0);

            size_t last = (N == 1 ? 0 : N % 2 == 0 ? 1 : 0);
            raze_assert(raze::vx::find_last_set(m) == last);
        }

        {
            Mask m(false);

            raze_assert(raze::vx::find_first_not_set(m) == 0);
            raze_assert(raze::vx::find_last_not_set(m) == 0);
        }

        {
            Mask m(true);

            raze_assert(raze::vx::find_first_not_set(m) == N);
            raze_assert(raze::vx::find_last_not_set(m) == N);
        }

        for (size_t i = 0; i < N; ++i) {
            Mask m(true);
            m[i] = false;

            raze_assert(raze::vx::find_first_not_set(m) == i);
            raze_assert(raze::vx::find_last_not_set(m) == (N - i - 1));
        }

        {
            Mask m(false);

            for (size_t pos = 0; pos < N; ++pos) {
                auto result = raze::vx::find_next_set(m, pos);
                raze_assert(result == N);
            }
        }

        {
            Mask m(true);

            for (size_t pos = 0; pos < N; ++pos) {
                auto found = raze::vx::find_next_set(m, pos);
                raze_assert(found == (pos + 1 < N ? pos + 1 : N));
            }
        }

        for (size_t bit = 0; bit < N; ++bit) {
            Mask m(false);
            m[bit] = true;

            for (size_t pos = 0; pos < N; ++pos) {
                auto r = raze::vx::find_next_set(m, pos);
                auto expected = expected_find_next(m, pos);
                raze_assert(r == expected);
            }
        }

        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });

            for (size_t pos = 0; pos < N; ++pos)
                raze_assert(raze::vx::find_next_set(m, pos) == expected_find_next(m, pos));
        }

        {
            auto m = make_pattern([](size_t i) { return i % 3 == 1; });

            for (size_t pos = 0; pos < N; ++pos)
                raze_assert(raze::vx::find_next_set(m, pos) == expected_find_next(m, pos));
        }

        {
            std::mt19937 rng(12345);

            for (int iter = 0; iter < 1000; ++iter) {
                Mask m(false);

                for (size_t i = 0; i < N; ++i)
                    m[i] = (rng() & 1);

                for (size_t pos = 0; pos < N; ++pos)
                    raze_assert(raze::vx::find_next_set(m, pos) == expected_find_next(m, pos));
            }
        }
    }

    void operator()() const {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        test_size<(_Width_ / (sizeof(_Type_) * 8)) + 1>();
        test_size<1>();
    }
};

int main() {
    test_all<variable_length_mask_tests>();
}