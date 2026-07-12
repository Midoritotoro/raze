#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct horizontal_minmax_tests {
    template <raze::u64 _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N], fallback[N];

        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_((i % 3 == 0) ? -(i + 5) : (i + 1));

        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        {
            auto hmin = raze::vx::fold(v, raze::vx::vertical_min);
            auto expected = (*std::min_element(arr, arr + N));
            raze_assert(hmin == expected);
        }
        {
            auto hmax = raze::vx::horizontal_max(v);
            auto expected = (*std::max_element(arr, arr + N));
            raze_assert(hmax == expected);
        }
        

    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) * 2>();
        test_size<1>();
    }
};

int main() {
    test_all<horizontal_minmax_tests>();
    return 0;
}
