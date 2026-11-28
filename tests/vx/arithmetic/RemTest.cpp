#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

#include <limits>
#include <vector>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::rem", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    if constexpr (!std::integral<T>) {
        static_assert(!requires(V a, V b) { a % b; });
        static_assert(!requires(V a, T s) { a % s; });
        static_assert(!requires(T s, V b) { s % b; });
        static_assert(!requires(V a, V b) { a %= b; });
        static_assert(!requires(V a, T s) { a %= s; });
        static_assert(!requires(V a, V b) { raze::vx::rem(a, b); });
        RTTS_EXPECT(true);
        return;
    }
    else {
        using lim = std::numeric_limits<T>;

        auto sanitize_pair = [](T& x, T& y) {
            if (y == T(0)) y = T(1);
            if constexpr (std::signed_integral<T>) {
                if (x == lim::min() && y == T(-1)) x = static_cast<T>(lim::min() + 1);
            }
        };

        auto sanitize_scalar = [](T* arr, T& s) {
            if (s == T(0)) s = T(1);
            if constexpr (std::signed_integral<T>) {
                if (s == T(-1)) {
                    for (size_t i = 0; i < N; ++i)
                        if (arr[i] == lim::min()) arr[i] = static_cast<T>(lim::min() + 1);
                }
            }
        };

        rtts::random::generator<T> gen(0x9E3779B97F4A7C15ULL + sizeof(T) * 41 + N);
        alignas(alignof(V)) T arrA[N], arrB[N], arrSrc[N];

        const auto any_domain = rtts::random::domain<T>::standard();
        const auto non_zero = rtts::random::domain<T>::non_zero();

        for (size_t trial = 0; trial < 500; ++trial) {
            gen.fill(arrA, any_domain);
            gen.fill(arrB, non_zero);
            gen.fill(arrSrc, any_domain);

            for (size_t i = 0; i < N; ++i)
                sanitize_pair(arrA[i], arrB[i]);

            T s = arrB[0];
            sanitize_scalar(arrA, s);

            T lhs_s = arrA[0];
            for (size_t i = 0; i < N; ++i) {
                if constexpr (std::signed_integral<T>) {
                    if (lhs_s == lim::min() && arrB[i] == T(-1))
                        lhs_s = static_cast<T>(lim::min() + 1);
                }
            }

            const V a = raze::vx::load<V>(arrA);
            const V b = raze::vx::load<V>(arrB);
            const V src = raze::vx::load<V>(arrSrc);
            const Mask m = rtts::simd::make_random_mask<Mask>();

            const V op_vv = a % b;
            const V op_vs = a % s;
            const V op_sv = lhs_s % b;
            const V op_vi = a % 3;
            const V op_iv = 100 % b;

            V eq_vv = a;
            eq_vv %= b;
            V eq_vs = a;
            eq_vs %= s;
            V eq_vi = a;
            eq_vi %= 3;

            const V r1 = raze::vx::rem(a, b);
            const V r2 = raze::vx::rem[m](a, b);
            const V r3 = raze::vx::rem[m, src](a, b);
            const V r4 = raze::vx::rem(a, s);
            const V r5 = raze::vx::rem[m](a, s);
            const V r6 = raze::vx::rem[m, src](a, s);
            const V r7 = raze::vx::rem(lhs_s, b);
            const V r8 = raze::vx::rem[m](lhs_s, b);
            const V r9 = raze::vx::rem[m, src](lhs_s, b);

            RTTS_ALL_VALIDATE_VECTOR(op_vv, [&](auto i) { return static_cast<T>(arrA[i] % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(op_vs, [&](auto i) { return static_cast<T>(arrA[i] % s); });
            RTTS_ALL_VALIDATE_VECTOR(op_sv, [&](auto i) { return static_cast<T>(lhs_s % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(op_vi, [&](auto i) { return static_cast<T>(arrA[i] % T(3)); });
            RTTS_ALL_VALIDATE_VECTOR(op_iv, [&](auto i) { return static_cast<T>(T(100) % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(eq_vv, [&](auto i) { return static_cast<T>(arrA[i] % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(eq_vs, [&](auto i) { return static_cast<T>(arrA[i] % s); });
            RTTS_ALL_VALIDATE_VECTOR(eq_vi, [&](auto i) { return static_cast<T>(arrA[i] % T(3)); });
            RTTS_ALL_VALIDATE_VECTOR(r1, [&](auto i) { return static_cast<T>(arrA[i] % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(r2, [&](auto i) { return m[i] ? static_cast<T>(arrA[i] % arrB[i]) : T(0); });
            RTTS_ALL_VALIDATE_VECTOR(r3, [&](auto i) { return m[i] ? static_cast<T>(arrA[i] % arrB[i]) : arrSrc[i]; });
            RTTS_ALL_VALIDATE_VECTOR(r4, [&](auto i) { return static_cast<T>(arrA[i] % s); });
            RTTS_ALL_VALIDATE_VECTOR(r5, [&](auto i) { return m[i] ? static_cast<T>(arrA[i] % s) : T(0); });
            RTTS_ALL_VALIDATE_VECTOR(r6, [&](auto i) { return m[i] ? static_cast<T>(arrA[i] % s) : arrSrc[i]; });
            RTTS_ALL_VALIDATE_VECTOR(r7, [&](auto i) { return static_cast<T>(lhs_s % arrB[i]); });
            RTTS_ALL_VALIDATE_VECTOR(r8, [&](auto i) { return m[i] ? static_cast<T>(lhs_s % arrB[i]) : T(0); });
            RTTS_ALL_VALIDATE_VECTOR(r9, [&](auto i) { return m[i] ? static_cast<T>(lhs_s % arrB[i]) : arrSrc[i]; });
        }

        std::vector<T> edge_x;
        std::vector<T> edge_d;
        if constexpr (std::signed_integral<T>) {
            edge_x = {
                T(0), T(1), T(-1), T(2), T(-2), T(3), T(-3), T(7), T(-7), T(127), T(-127),
                lim::max(), static_cast<T>(lim::max() - 1), lim::min(), static_cast<T>(lim::min() + 1)
            };
            edge_d = {
                T(1), T(-1), T(2), T(-2), T(3), T(-3), T(4), T(-4), T(7), T(-7), T(13), T(-13),
                lim::max(), static_cast<T>(lim::max() - 1), lim::min(), static_cast<T>(lim::min() + 1)
            };
        }
        else {
            edge_x = {
                T(0), T(1), T(2), T(3), T(7), T(127), T(128), T(255),
                lim::max(), static_cast<T>(lim::max() - 1), static_cast<T>(lim::max() / 2)
            };
            edge_d = {
                T(1), T(2), T(3), T(4), T(7), T(13), T(127), T(128), T(255),
                lim::max(), static_cast<T>(lim::max() - 1), static_cast<T>(lim::max() / 2)
            };
        }

        for (T d : edge_d) {
            for (size_t shift = 0; shift < edge_x.size(); ++shift) {
                for (size_t i = 0; i < N; ++i) {
                    arrA[i] = edge_x[(i + shift) % edge_x.size()];
                    arrB[i] = d;
                    sanitize_pair(arrA[i], arrB[i]);
                }
                const V a = raze::vx::load<V>(arrA);
                const V b = raze::vx::load<V>(arrB);
                RTTS_ALL_VALIDATE_VECTOR(a % d, [&](auto i) { return static_cast<T>(arrA[i] % d); });
                RTTS_ALL_VALIDATE_VECTOR(a % b, [&](auto i) { return static_cast<T>(arrA[i] % arrB[i]); });
            }
        }

        const Mask alt = rtts::simd::make_alternating_mask<Mask>();
        for (size_t i = 0; i < N; ++i) {
            arrA[i] = alt[i] ? T(17 + i) : lim::min();
            if constexpr (std::signed_integral<T>)
                arrB[i] = alt[i] ? T(5) : ((i % 2 == 0) ? T(0) : T(-1));
            else
                arrB[i] = alt[i] ? T(5) : T(0);
            arrSrc[i] = T(77 + i);
        }
        const V safe_a = raze::vx::load<V>(arrA);
        const V safe_b = raze::vx::load<V>(arrB);
        const V safe_src = raze::vx::load<V>(arrSrc);

        RTTS_ALL_VALIDATE_VECTOR(raze::vx::rem[alt](safe_a, safe_b), [&](auto i) {
            return alt[i] ? static_cast<T>(arrA[i] % arrB[i]) : T(0);
        });
        RTTS_ALL_VALIDATE_VECTOR(raze::vx::rem[alt, safe_src](safe_a, safe_b), [&](auto i) {
            return alt[i] ? static_cast<T>(arrA[i] % arrB[i]) : arrSrc[i];
        });
    }
};

RAZE_TEST_NAMESPACE_END

