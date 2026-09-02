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

        {
            auto m = raze::vx::first_n(0, raze::options::as(Mask{}));
            for (size_t i = 0; i < N; ++i)
                raze_assert(!m[i]);
        }

        {
            auto m = raze::vx::first_n(N, raze::options::as(Mask{}));
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i]);
        }

        {
            for (size_t k = 0; k <= N; ++k) {
                auto m = raze::vx::first_n(k, raze::options::as(Mask{}));
                for (size_t i = 0; i < N; ++i)
                    raze_assert(m[i] == (i < k));
            }
        }

        {
            std::mt19937 rng(98765);
            for (int iter = 0; iter < 500; ++iter) {
                size_t k = rng() % N;
                auto m = raze::vx::first_n(k, raze::options::as(Mask{}));
                for (size_t i = 0; i < N; ++i)
                    raze_assert(m[i] == (i < k));
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