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

        {
            Mask t(true);
            Mask f(false);
            
            raze_assert(raze::vx::any_of(t));
            raze_assert(raze::vx::all_of(t));
            raze_assert(raze::vx::none_of(f));
        }

        {
            bool src[N];
            bool dst[N];

            for (size_t i = 0; i < N; ++i)
                src[i] = (i % 3 == 0);

            Mask m(src);
            m.copy_to(dst);

            for (size_t i = 0; i < N; ++i)
                raze_assert(dst[i] == src[i]);
        }

        {
            Mask m(false);

            if (N) {
                m[0] = true;
                m[N - 1] = true;

                raze_assert(m[0]);
                raze_assert(m[N - 1]);

                for (size_t i = 1; i + 1 < N; ++i)
                    raze_assert(!m[i]);
            }
        }

  /*      {
            auto a = make_pattern([](size_t i) { return i % 2 == 0; });
            auto b = make_pattern([](size_t i) { return i % 3 == 0; });

            auto c_and = a & b;
            auto c_or = a | b;
            auto c_xor = a ^ b;
            auto c_not = !a;

            for (size_t i = 0; i < N; ++i) {
                bool av = a[i];
                bool bv = b[i];

                raze_assert(c_and[i] == (av && bv));
                raze_assert(c_or[i] == (av || bv));
                raze_assert(c_xor[i] == (av ^ bv));
                raze_assert(c_not[i] == (!av));
            }
        }

        {
            Mask t(true);
            Mask f(false);

            raze_assert(raze::vx::all_of(t));
            raze_assert(!raze::vx::none_of(t));
            raze_assert(!raze::vx::some_of(t));

            raze_assert(raze::vx::none_of(f));
            raze_assert(!raze::vx::any_of(f));
            raze_assert(raze::vx::some_of(f));

            Mask m(false);
            if (N) m[0] = true;

            raze_assert(raze::vx::any_of(m));
            raze_assert(raze::vx::some_of(m));
            raze_assert(!raze::vx::all_of(m));
        }*/
        /*
        {
            Mask m(false);

            raze_assert(raze::vx::count_set(m) == 0);
            raze_assert(!raze::vx::any_of(m));
            raze_assert(!raze::vx::all_of(m));
            raze_assert(raze::vx::find_first_set(m) == N);
            raze_assert(raze::vx::find_last_set(m) == N);
        }

        {
            Mask m(true);

            raze_assert(raze::vx::count_set(m) == N);
            raze_assert(raze::vx::any_of(m));
            raze_assert(raze::vx::all_of(m));
            raze_assert(raze::vx::find_first_set(m) == 0);
            raze_assert(raze::vx::find_last_set(m) == 0);
        }

        for (size_t i = 0; i < N; ++i) {
            Mask m(false);
            m[i] = true;

            raze_assert(raze::vx::count_set(m) == 1);
            raze_assert(raze::vx::any_of(m));
            raze_assert(!raze::vx::all_of(m));
            raze_assert(raze::vx::find_first_set(m) == i);
            raze_assert(raze::vx::find_last_set(m) == (N - i - 1));
        }

        {
            auto m = make_pattern([](size_t i) { return i % 2 == 0; });

            size_t expected = (N + 1) / 2;

            raze_assert(raze::vx::count_set(m) == expected);
            raze_assert(raze::vx::any_of(m));
            raze_assert(!raze::vx::all_of(m));
            raze_assert(raze::vx::find_first_set(m) == 0);

            size_t last = (N % 2 == 0 ? 1 : N - 1);
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

            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(!raze::vx::is_contiguous(m, n, k));
        }

        {
            Mask m(true);

            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(raze::vx::is_contiguous(m, n, k));
        }

        for (size_t pos = 0; pos < N; ++pos) {
            Mask m(false);
            m[pos] = true;

            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(raze::vx::is_contiguous(m, n, k) == (n == pos && k == pos));
        }

        {
            std::mt19937 rng(123456);

            for (int iter = 0; iter < 200; ++iter) {
                Mask m(false);

                for (size_t i = 0; i < N; ++i)
                    m[i] = (rng() & 1);

                for (size_t n = 0; n < N; ++n) {
                    for (size_t k = n; k < N; ++k) {

                        bool expected = true;
                        for (size_t i = n; i <= k; ++i)
                            if (!m[i]) expected = false;

                        raze_assert(raze::vx::is_contiguous(m, n, k) == expected);
                    }
                }
            }
        }*/
    }

    void operator()() const {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        //test_size<1>();
        //test_size<7>();
        //test_size<17>();
        //test_size<31>();
    }
};

int main() {
    test_all<variable_length_mask_tests>();
}