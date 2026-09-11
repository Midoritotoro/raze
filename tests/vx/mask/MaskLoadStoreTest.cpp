#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RTTS_CASE_TPL("raze::vx::simd_mask<T>::copy_to, raze::vx::simd_mask<T>::copy_from", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    bool src[N];
    bool dst[N];

    for (size_t i = 0; i < N; ++i)
        src[i] = (i % 3 == 0);

    Mask m(src);
    m.copy_to(dst);

    for (size_t i = 0; i < N; ++i)
        RTTS_EXPECT(dst[i] == src[i]);
};