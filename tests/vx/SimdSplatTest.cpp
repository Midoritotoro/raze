#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32       _Width_>
struct splat_tests {
    template <raze::sizetype _Size_, raze::sizetype _Index_>
    void test_index() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;

        static constexpr size_t N = Simd::size();
        alignas(64) _Type_ arr[N];

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_(i + 1);

        Simd v = raze::vx::load<Simd>(arr);
        auto r = raze::vx::splat<_Index_>(v);

        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[_Index_]);
    }

    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;

        [&] <raze::sizetype... I>(std::integer_sequence<raze::sizetype, I...>) {
            (test_index<_Size_, I>(), ...);
        }(std::make_integer_sequence<raze::sizetype, Simd::size()>{});
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        //etest_size<_Width_ / (sizeof(_Type_) * 8) * 2>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();
    }
};

int main() {
    test_all<splat_tests>();
    return 0;
}