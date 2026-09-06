#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::bit_and", rtts::simd::all_simd_infos)
< class Simd > (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    alignas(alignof(V)) T arrA[N], arrB[N], arrSrc[N];

    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T(N - i);
        arrSrc[i] = T(100 + i);
    }

    V a = raze::vx::load<V>(arrA);
    V b = raze::vx::load<V>(arrB);
    V src = raze::vx::load<V>(arrSrc);

    Mask m = rtts::simd::make_random_mask<Mask>();

    auto scalar_eval = [&] (size_t i) {
        using CommonIntegralType = typename raze::IntegerForSizeof<T>::Unsigned;
        if constexpr (std::floating_point<T>) return raze::math::bit_cast<T>(CommonIntegralType(
            raze::math::bit_cast<CommonIntegralType>(arrA[i])) & raze::math::bit_cast<CommonIntegralType>(arrB[i]));
        else return T(arrA[i] & arrB[i]);
        };

    auto r1 = raze::vx::bit_and(a, b);
    auto r2 = raze::vx::bit_and[m](a, b);
    auto r3 = raze::vx::bit_and[m, src](a, b);

    RTTS_ALL_VALIDATE_BITS(r1, [&](auto i) { return scalar_eval(i); });
    RTTS_ALL_VALIDATE_BITS(r2, [&](auto i) { return m[i] ? scalar_eval(i) : T(0); });
    RTTS_ALL_VALIDATE_BITS(r3, [&](auto i) { return m[i] ? scalar_eval(i) : arrSrc[i]; });
};