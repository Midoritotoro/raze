#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <bit>
#include <cmath>
#include <algorithm>
#include <vector>

template <class V, class Mask, class T, size_t N, raze::u8 Op>
void test_ternarylogic_single_op() {
    alignas(alignof(V)) T arrA[N], arrB[N], arrC[N], arrSrc[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T((i + 1) * 3);
        arrC[i] = T((i + 1) * 7);
        arrSrc[i] = T(100 + i);
    }

    V a = raze::vx::load<V>(arrA);
    V b = raze::vx::load<V>(arrB);
    V c = raze::vx::load<V>(arrC);
    V src = raze::vx::load<V>(arrSrc);

    auto scalar_eval = [](raze::u8 mask, T valA, T valB, T valC) {
        using U = typename raze::IntegerForSizeof<T>::Unsigned;
        U uA = std::bit_cast<U>(valA);
        U uB = std::bit_cast<U>(valB);
        U uC = std::bit_cast<U>(valC);
        U result = 0;

        for (size_t i = 0; i < sizeof(T) * 8; ++i) {
            bool xa = (uA >> i) & 1;
            bool xb = (uB >> i) & 1;
            bool xc = (uC >> i) & 1;
            raze::u8 idx = (raze::u8(xa) << 2) | (raze::u8(xb) << 1) | raze::u8(xc);
            if ((mask >> idx) & 1) {
                result |= (U(1) << i);
            }
        }
        return result;
    };

    auto verify = [&](const V& r, auto&& expected_fn) {
        using U = typename raze::IntegerForSizeof<T>::Unsigned;
        bool res = true;
        for (size_t i = 0; i < N; ++i) {
            U expected = expected_fn(i);
            res = (std::bit_cast<U>(T(r[i])) == expected);
            if (!res) break;
        }
        RTTS_EXPECT(res);
    };

    using imm_t = std::integral_constant<raze::u8, Op>;

    auto run_tests = [&](const Mask& m) {
        V r1 = raze::vx::ternarylogic(a, b, c, imm_t{});
        verify(r1, [&](size_t i) { return scalar_eval(Op, arrA[i], arrB[i], arrC[i]); });

        V r2 = raze::vx::ternarylogic[m, src](a, b, c, imm_t{});
        verify(r2, [&](size_t i) { return m[i] ? scalar_eval(Op, arrA[i], arrB[i], arrC[i]) : std::bit_cast<typename raze::IntegerForSizeof<T>::Unsigned>(arrSrc[i]); });
    };

    for (auto i = 0; i < std::min(static_cast<int>(std::pow(2, N)), 100); ++i) {
        run_tests(rtts::simd::make_random_mask<Mask>());
    }
}

template <class V, class Mask, class T, size_t N, std::size_t... I>
void test_ternarylogic_all_ops_impl(std::index_sequence<I...>) {
    (test_ternarylogic_single_op<V, Mask, T, N, static_cast<raze::u8>(I)>(), ...);
}

template <class V, class Mask, class T, size_t N>
void test_ternarylogic_all_ops() {
    test_ternarylogic_all_ops_impl<V, Mask, T, N>(std::make_index_sequence<8>{});
}

RTTS_CASE_TPL("raze::vx::ternarylogic", rtts::simd::all_simd_infos)
<class SimdInfo> (rtts::type<SimdInfo>) {
    using V = typename SimdInfo::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    test_ternarylogic_all_ops<V, Mask, T, N>();
};