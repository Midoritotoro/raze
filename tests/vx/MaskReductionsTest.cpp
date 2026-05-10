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
            Mask t(true);
            Mask f(false);

            raze_assert(raze::vx::all_of(t));
            raze_assert(!raze::vx::none_of(t));
            raze_assert(!raze::vx::some_of(t));

            raze_assert(raze::vx::none_of(f));
            raze_assert(!raze::vx::any_of(f));
            raze_assert(!raze::vx::some_of(f));

            Mask m(false);
            m[0] = true;

            raze_assert(raze::vx::any_of(m));
            N == 1 ? raze_assert(!raze::vx::some_of(m)) : raze_assert(raze::vx::some_of(m));
            N == 1 ? raze_assert(raze::vx::all_of(m)) : raze_assert(!raze::vx::all_of(m));
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
        }
    }

    void operator()() const {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        test_size<1>();
    }
};

int main() {
    test_all<variable_length_mask_tests>();
}