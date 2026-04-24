#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct variable_length_basics_tests {
    template <raze::uint32 _N_>
    void test_size() const {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        constexpr size_t N = _N_;
        constexpr size_t PAD = 4;

        Simd v1;
        Simd v2(5);

        for (size_t i = 0; i < N; ++i)
            raze_assert(v2[i] == 5);

        alignas(64) _Type_ arr[N];
        std::iota(arr, arr + N, 1);

        Simd v3; v3.copy_from(arr);
        for (size_t i = 0; i < N; ++i)
            raze_assert(v3[i] == arr[i]);

        Simd v4(v3);
        Simd v5 = v3;

        for (size_t i = 0; i < N; ++i) {
            raze_assert(v4[i] == arr[i]);
            raze_assert(v5[i] == arr[i]);
        }

        Simd v;
        v.fill(42);

        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == 42);

        v[0] = 99;
        raze_assert(v[0] == 99);
    }

    void operator()() const {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        test_size<1>();
        test_size<7>();
        test_size<17>();
        test_size<31>();
    }
};

int main() {
    test_all<variable_length_basics_tests>();
    return 0;
}
