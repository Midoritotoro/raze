#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>


template <typename _Simd_>
void mask_compress_any(
    const typename _Simd_::value_type*  a,
    const typename _Simd_::value_type*  src,
    typename _Simd_::value_type*        dst,
    typename _Simd_::mask_type          mask)
{
    constexpr auto N = _Simd_::size();
    int m = 0;

    for (int j = 0; j < N; ++j)
        if (!mask[j])
            dst[m++] = a[j];

    for (int i = m; i < N; ++i)
        dst[i] = src[i];
}

template <class _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct compress_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ src[N];
        for (size_t i = 0; i < N; ++i)
            src[i] = _Type_(i + 1);

        Simd v = raze::vx::load<Simd>(src);
        
        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            Mask mask = make_random_mask<Mask>();

            alignas(64) _Type_ dst[N], expected[N];
            mask_compress_any<Simd>(src, src, expected, mask);

            raze::vx::compress_store(dst, v, mask);
			const auto count = raze::vx::count_set(!mask);
            raze_assert(std::equal(dst, dst + count, expected, expected + count));
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 3>();
        test_size<1>();
    }
};

int main() {
    test_all<compress_tests>();
    return 0;
}
