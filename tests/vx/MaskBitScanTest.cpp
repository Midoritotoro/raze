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
        }
        {
            Mask m(true);
            raze_assert(raze::vx::count_set(m) == N);
        }
        for (size_t i = 0; i < N; ++i) {
            Mask m(false);
            m[i] = true;
            raze_assert(raze::vx::count_set(m) == 1);
        }
        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            raze_assert(raze::vx::count_set(m) == (N + 1) / 2);
        }

        {
            Mask all_true(true);
            Mask all_false(false);

            raze_assert(raze::vx::count_set[all_false](all_true) == 0);
            raze_assert(raze::vx::count_set[all_true](all_false) == 0);
            raze_assert(raze::vx::count_set[all_true](all_true) == N);

            for (size_t i = 0; i < N; ++i) {
                Mask m(false);
                m[i] = true;
                raze_assert(raze::vx::count_set[all_true](m) == 1);
                raze_assert(raze::vx::count_set[m](all_true) == 1);
                raze_assert(raze::vx::count_set[m](all_false) == 0);
            }

            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            raze_assert(raze::vx::count_set[all_true](m) == (N + 1) / 2);

            Mask half(false);
            for (size_t i = 0; i < N / 2; ++i) half[i] = true;
            raze_assert(raze::vx::count_set[half](all_true) == N / 2);
        }

        {
            Mask m(false);
            raze_assert(raze::vx::find_first_set(m) == N);
        }
        {
            Mask m(true);
            raze_assert(raze::vx::find_first_set(m) == 0);
        }
        for (size_t i = 0; i < N; ++i) {
            Mask m(false);
            m[i] = true;
            raze_assert(raze::vx::find_first_set(m) == i);
        }
        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            raze_assert(raze::vx::find_first_set(m) == 0);
        }

        {
            Mask all_true(true);
            Mask all_false(false);

            raze_assert(raze::vx::find_first_set[all_false](all_true) == N);
            raze_assert(raze::vx::find_first_set[all_true](all_false) == N);
            raze_assert(raze::vx::find_first_set[all_true](all_true) == 0);

            for (size_t i = 0; i < N; ++i) {
                Mask m(false);
                m[i] = true;
                raze_assert(raze::vx::find_first_set[all_true](m) == i);
                raze_assert(raze::vx::find_first_set[m](all_true) == i);
                raze_assert(raze::vx::find_first_set[m](all_false) == N);
            }

            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            raze_assert(raze::vx::find_first_set[all_true](m) == 0);

            if (N > 2) {
                Mask skip_first(false);
                for (size_t i = 2; i < N; ++i) skip_first[i] = true;
                raze_assert(raze::vx::find_first_set[skip_first](m) == 2);
            }
        }

        {
            Mask m(false);
            raze_assert(raze::vx::find_last_set(m) == N);
        }
        {
            Mask m(true);
            raze_assert(raze::vx::find_last_set(m) == 0);
        }
        for (size_t i = 0; i < N; ++i) {
            Mask m(false);
            m[i] = true;
            raze_assert(raze::vx::find_last_set(m) == (N - i - 1));
        }
        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            size_t last = (N == 1 ? 0 : N % 2 == 0 ? 1 : 0);
            raze_assert(raze::vx::find_last_set(m) == last);
        }

        {
            Mask all_true(true);
            Mask all_false(false);

            raze_assert(raze::vx::find_last_set[all_false](all_true) == N);
            raze_assert(raze::vx::find_last_set[all_true](all_false) == N);
            raze_assert(raze::vx::find_last_set[all_true](all_true) == 0);

            for (size_t i = 0; i < N; ++i) {
                Mask m(false);
                m[i] = true;
                raze_assert(raze::vx::find_last_set[all_true](m) == (N - i - 1));
                raze_assert(raze::vx::find_last_set[m](all_true) == (N - i - 1));
                raze_assert(raze::vx::find_last_set[m](all_false) == N);
            }

            auto m = make_pattern([](size_t i) { return i % 2 == 0; });
            size_t last = (N == 1 ? 0 : N % 2 == 0 ? 1 : 0);
            raze_assert(raze::vx::find_last_set[all_true](m) == last);

            if (N > 2) {
                Mask skip_last(false);
                for (size_t i = 0; i < N - 2; ++i) skip_last[i] = true;
                size_t expected = N;
                for (auto i = m.size() - 1; i >= 0; --i) {
                    if (m[i] && skip_last[i]) {
                        expected = N - 1 - i;
                        break;
                    }
                }
               
                auto r = raze::vx::find_last_set[skip_last](m);
                raze_assert(r == expected);
            }
        }

        {
            Mask m(false);
            raze_assert(raze::vx::find_first_not_set(m) == 0);
        }
        {
            Mask m(true);
            raze_assert(raze::vx::find_first_not_set(m) == N);
        }
        for (size_t i = 0; i < N; ++i) {
            Mask m(true);
            m[i] = false;
            raze_assert(raze::vx::find_first_not_set(m) == i);
        }

        {
            Mask all_true(true);
            Mask all_false(false);

            raze_assert(raze::vx::find_first_not_set[all_false](all_true) == N);
            raze_assert(raze::vx::find_first_not_set[all_true](all_true) == N);
            raze_assert(raze::vx::find_first_not_set[all_true](all_false) == 0);

            for (size_t i = 0; i < N; ++i) {
                Mask m(true);
                m[i] = false;
                raze_assert(raze::vx::find_first_not_set[all_true](m) == i);

                Mask single(false);
                single[i] = true;
                raze_assert(raze::vx::find_first_not_set[single](all_true) == N);
                raze_assert(raze::vx::find_first_not_set[single](all_false) == i);
            }
        }

        {
            Mask m(false);
            raze_assert(raze::vx::find_last_not_set(m) == 0);
        }
        {
            Mask m(true);
            raze_assert(raze::vx::find_last_not_set(m) == N);
        }
        for (size_t i = 0; i < N; ++i) {
            Mask m(true);
            m[i] = false;
            raze_assert(raze::vx::find_last_not_set(m) == (N - i - 1));
        }

        {
            Mask all_true(true);
            Mask all_false(false);

            raze_assert(raze::vx::find_last_not_set[all_false](all_true) == N);
            raze_assert(raze::vx::find_last_not_set[all_true](all_true) == N);
            raze_assert(raze::vx::find_last_not_set[all_true](all_false) == 0);

            for (size_t i = 0; i < N; ++i) {
                Mask m(true);
                m[i] = false;
                raze_assert(raze::vx::find_last_not_set[all_true](m) == (N - i - 1));

                Mask single(false);
                single[i] = true;
                raze_assert(raze::vx::find_last_not_set[single](all_true) == N);
                raze_assert(raze::vx::find_last_not_set[single](all_false) == (N - i - 1));
            }
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

        {
            std::mt19937 rng(54321);

            for (int iter = 0; iter < 1000; ++iter) {
                Mask main(false), extra(false);
                for (size_t i = 0; i < N; ++i) {
                    main[i] = (rng() & 1);
                    extra[i] = (rng() & 1);
                }

                size_t expected_count = 0;
                size_t expected_first = N;
                size_t expected_last = N;
                size_t expected_first_not = N;
                size_t expected_last_not = N;

                for (size_t i = 0; i < N; ++i) {
                    if (extra[i]) {
                        if (main[i]) {
                            ++expected_count;
                            if (expected_first == N) expected_first = i;
                            expected_last = i;
                        }
                        else {
                            if (expected_first_not == N) expected_first_not = i;
                            expected_last_not = i;
                        }
                    }
                }

                size_t expected_last_rev = (expected_last == N) ? N : (N - expected_last - 1);
                size_t expected_last_not_rev = (expected_last_not == N) ? N : (N - expected_last_not - 1);

                raze_assert(raze::vx::count_set[extra](main) == expected_count);
                raze_assert(raze::vx::find_first_set[extra](main) == expected_first);
                raze_assert(raze::vx::find_last_set[extra](main) == expected_last_rev);
                raze_assert(raze::vx::find_first_not_set[extra](main) == expected_first_not);
                raze_assert(raze::vx::find_last_not_set[extra](main) == expected_last_not_rev);
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