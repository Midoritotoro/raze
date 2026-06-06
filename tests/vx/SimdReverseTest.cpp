#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct reverse_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N];

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_(i + 1);

        Simd v = raze::vx::load<Simd>(arr);
        auto r = raze::vx::reverse(v);

        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[N - 1 - i]);
    }

    void operator()() {
        //test_size<_Width_ / (sizeof(_Type_) * 8)>();
        //test_size<_Width_ / (sizeof(_Type_) * 8) * 2>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();

    }
};

int main() {
    test_all<reverse_tests>();
    return 0;
}
