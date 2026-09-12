#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RTTS_CASE_TPL("raze::vx::simd_mask<T> bitwise operations", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    auto make_pattern = [=](auto f) {
        Mask m(false);
        for (size_t i = 0; i < N; ++i)
            m[i] = f(i);
        return m;
    };

    auto a = make_pattern([=](size_t i) { return i % 2 == 0; });
    auto b = make_pattern([=](size_t i) { return i % 3 == 0; });

    auto c_and = a & b;
    auto c_or = a | b;
    auto c_xor = a ^ b;
    auto c_not = !a;

    for (size_t i = 0; i < N; ++i) {
        bool av = a[i];
        bool bv = b[i];

        RTTS_EXPECT(c_and[i] == (av && bv));
        RTTS_EXPECT(c_or[i] == (av || bv));
        RTTS_EXPECT(c_xor[i] == (av ^ bv));
        RTTS_EXPECT(c_not[i] == (!av));
    }
};