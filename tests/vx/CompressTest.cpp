#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>


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

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct compress_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ src[N];
        for (size_t i = 0; i < N; ++i)
            src[i] = _Type_(i + 1);

        Simd v; v.copy_from(src);
        
        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            Mask mask = make_random_mask<Mask>();

            alignas(64) _Type_ dst[N], expected[N];
            mask_compress_any<Simd>(src, src, expected, mask);

            raze::vx::compress_store(dst, v, mask);
            raze_assert(std::equal(dst, dst + N, expected, expected + N));
        }
    }
};

int main() {
    test_all<compress_tests>();
    return 0;
}
