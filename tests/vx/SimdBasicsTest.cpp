#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct basics_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        Simd v1;
        Simd v2(5);

        for (size_t i = 0; i < N; ++i)
            raze_assert(v2[i] == 5);

        alignas(64) _Type_ arr[N];
        std::iota(arr, arr + N, 1);

        Simd v3 = raze::vx::load<Simd>(arr);
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
};

int main() {
    test_all<basics_tests>();
    return 0;
}
