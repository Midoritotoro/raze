#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::bit_or", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    constexpr size_t N = V::size();

    T arrA[N], arrB[N];

    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T(N - i);
    }

    V a = raze::vx::load<V>(arrA);
    V b = raze::vx::load<V>(arrB);

    RTTS_ALL_VALIDATE_BITS(a | b, [=] (auto i) {
        using CommonIntegralType = typename raze::IntegerForSizeof<T>::Unsigned;
        if constexpr (std::floating_point<T>) return raze::math::bit_cast<T>(
            raze::math::bit_cast<CommonIntegralType>(arrA[i]) | raze::math::bit_cast<CommonIntegralType>(arrB[i]));
        else return arrA[i] | arrB[i]; 
    });
};
