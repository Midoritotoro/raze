#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <algorithm>
#include <cmath>

RAZE_TEST_NAMESPACE_BEGIN

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
}

RTTS_CASE_TPL("raze::vx::compress_store", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(std::hardware_constructive_interference_size) T src[N];
    for (size_t i = 0; i < N; ++i)
        src[i] = T(i + 1);
    
    V v = raze::vx::load<V>(src);
    
    for (int i = 0; i < 1000; ++i) {
        Mask mask = rtts::simd::make_random_mask<Mask>();

        alignas(std::hardware_constructive_interference_size) T dst[N], expected[N];
        std::ranges::fill(dst, 0);
        std::ranges::fill(expected, 0);

        mask_compress_any<V>(src, src, expected, mask);

        auto new_dest = raze::vx::compress_store[raze::vx::unsafe](dst, v, mask);
        const auto count = raze::vx::count_set(!mask);
        
        RTTS_EXPECT(std::equal(dst, dst + count, expected, expected + count) && (new_dest == (dst + count)));
    }

    for (int i = 0; i < 1000; ++i) {
        Mask mask = rtts::simd::make_random_mask<Mask>();

        alignas(std::hardware_constructive_interference_size) T dst[N], expected[N];
        std::ranges::fill(dst, 0);
        std::ranges::fill(expected, 0);
        mask_compress_any<V>(src, src, expected, mask);

        auto new_dest = raze::vx::compress_store(dst, v, mask);
        const auto count = raze::vx::count_set(!mask);

        RTTS_EXPECT(std::equal(dst, dst + mask.size(), expected, expected + mask.size()) && (new_dest == (dst + count)));
    }
};

RAZE_TEST_NAMESPACE_END
