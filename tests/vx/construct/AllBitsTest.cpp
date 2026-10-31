#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/BitCast.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::all_bits", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrSrc[N];
    alignas(alignof(V)) T arrR1[N], arrR2[N], arrR3[N];

    for (size_t i = 0; i < N; ++i) {
        arrSrc[i] = T(100 + i);
    }

    V src = raze::vx::load<V>(arrSrc);
    Mask m = rtts::simd::make_random_mask<Mask>();

    using CommonIntegralType = typename raze::IntegerForSizeof<T>::Unsigned;
    using SignedInt = std::make_signed_t<CommonIntegralType>;

    auto r1 = raze::vx::all_bits<V>();
    auto r2 = raze::vx::all_bits<V>[m]();
    auto r3 = raze::vx::all_bits<V>[m, src]();

    raze::vx::store(arrR1, r1);
    raze::vx::store(arrR2, r2);
    raze::vx::store(arrR3, r3);

    constexpr SignedInt all_ones = SignedInt(-1);

    for (size_t i = 0; i < N; ++i) {
        RTTS_EXPECT(raze::math::bit_cast<SignedInt>(arrR1[i]) == all_ones);

        if (m[i]) {
            RTTS_EXPECT(raze::math::bit_cast<SignedInt>(arrR2[i]) == all_ones);
            RTTS_EXPECT(raze::math::bit_cast<SignedInt>(arrR3[i]) == all_ones);
        }
        else {
            RTTS_EXPECT(raze::math::bit_cast<SignedInt>(arrR2[i]) == SignedInt(0));
            RTTS_EXPECT(raze::math::bit_cast<SignedInt>(arrR3[i]) == raze::math::bit_cast<SignedInt>(arrSrc[i]));
        }
    }
};
RAZE_TEST_NAMESPACE_END
