#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <algorithm>
#include <cmath>

template <class V>
void mask_compress_any(const typename V::value_type*  a,
    const typename V::value_type* src, typename V::value_type* dst,
    typename V::mask_type mask)
{
    constexpr auto N = V::size();
    int m = 0;

    for (int j = 0; j < N; ++j)
        if (!mask[j])
            dst[m++] = a[j];

    for (int i = m; i < N; ++i)
        dst[i] = src[i];
}

RTTS_CASE_TPL("raze::vx::compress_store", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(64) T src[N];
    for (size_t i = 0; i < N; ++i)
        src[i] = T(i + 1);

    V v = raze::vx::load<V>(src);
    
    int iterations = std::min(int(std::pow(2, N)), 10000);
    for (int i = 0; i < iterations; ++i) {
        Mask mask = rtts::simd::make_random_mask<Mask>();

        alignas(64) T dst[N], expected[N];
        mask_compress_any<V>(src, src, expected, mask);

        raze::vx::compress_store(dst, v, mask);
        const auto count = raze::vx::count_set(!mask);
        
        RTTS_EXPECT(std::equal(dst, dst + count, expected, expected + count));
    }
};
