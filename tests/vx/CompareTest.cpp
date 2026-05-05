#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct compare_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N];
        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_(i + 1);
            arrB[i] = _Type_(N - i);
        }

        Simd a; a.copy_from(arrA);
        Simd b; b.copy_from(arrB);

        {
            auto m = (a == a);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == true);
        }

        {
            auto m = (a != b);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == (arrA[i] != arrB[i]));
        }

        {
            auto m = (a < b);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == (arrA[i] < arrB[i]));
        }

        {
            auto m = (a > b);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == (arrA[i] > arrB[i]));
        }

        {
            auto m = (a <= b);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == (arrA[i] <= arrB[i]));
        }

        {
            auto m = (a >= b);
            for (size_t i = 0; i < N; ++i)
                raze_assert(m[i] == (arrA[i] >= arrB[i]));
        }
    }
};

int main() {
    test_all<compare_tests>();
    return 0;
}
