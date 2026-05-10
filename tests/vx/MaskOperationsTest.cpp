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

        {
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
    }

    void operator()() const {
        //test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        test_size<1>();
        //test_size<7>();
        //test_size<17>();
    }
};

int main() {
    test_all<variable_length_mask_tests>();
}