#pragma once

#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

namespace rtts::vx_fp {
    template <class T>
    inline std::vector<T> classify_values() {
        using lim = std::numeric_limits<T>;
        const T qnan = lim::quiet_NaN();
        const T neg_qnan = std::copysign(qnan, T(-1.0));
        const T snan = lim::signaling_NaN();
        const T neg_snan = std::copysign(snan, T(-1.0));
        const T inf = lim::infinity();
        const T min_norm = lim::min();
        const T denorm = lim::denorm_min();
        const T max_sub = std::nextafter(min_norm, T(0.0));

        return {
            T(0.0), T(-0.0), T(1.0), T(-1.0), T(0.5), T(-0.5), T(42.25), T(-42.25),
            min_norm, -min_norm, lim::max(), lim::lowest(),
            denorm, -denorm, max_sub, -max_sub,
            inf, -inf, qnan, neg_qnan, snan, neg_snan
        };
    }

    template <class T>
    inline std::vector<T> round_values() {
        using lim = std::numeric_limits<T>;
        const T prev_half = std::nextafter(T(0.5), T(0.0));
        const T next_half = std::nextafter(T(0.5), T(1.0));
        const T prev_1p5 = std::nextafter(T(1.5), T(0.0));
        const T big_pow2 = (sizeof(T) == 8) ? T(4503599627370496.0) : T(8388608.0f);
        const T half_pow2 = big_pow2 * T(0.5);

        return {
            T(0.0), T(-0.0), T(0.1), T(-0.1),
            prev_half, -prev_half, T(0.5), T(-0.5), next_half, -next_half,
            T(0.9), T(-0.9), T(1.0), T(-1.0), prev_1p5, -prev_1p5,
            T(1.5), T(-1.5), T(2.5), T(-2.5), T(3.5), T(-3.5), T(123.45), T(-123.45),
            half_pow2, -half_pow2, half_pow2 + T(0.5), -(half_pow2 + T(0.5)),
            big_pow2 - T(1.0), -(big_pow2 - T(1.0)), big_pow2 - T(0.5), -(big_pow2 - T(0.5)),
            big_pow2, -big_pow2, big_pow2 + T(1.0), -(big_pow2 + T(1.0)),
            big_pow2 * T(2.0), -(big_pow2 * T(2.0)),
            lim::denorm_min(), -lim::denorm_min(), lim::min(), -lim::min(),
            lim::max(), lim::lowest(), lim::infinity(), -lim::infinity(),
            lim::quiet_NaN(), std::copysign(lim::quiet_NaN(), T(-1.0))
        };
    }

    template <class T>
    inline std::vector<T> sqrt_rcp_values() {
        using lim = std::numeric_limits<T>;
        return {
            T(0.0), T(-0.0), T(1.0), T(4.0), T(9.0), T(0.25), T(2.0), T(3.0),
            T(-1.0), T(-4.0), T(-0.25),
            lim::min(), lim::max(), lim::denorm_min(),
            lim::infinity(), -lim::infinity(), lim::quiet_NaN()
        };
    }

    template <class T>
    inline bool bits_match(T actual, T expected) noexcept {
        if (std::isnan(expected)) return std::isnan(actual);
        return rtts::detail::to_bits(actual) == rtts::detail::to_bits(expected);
    }

    template <class T>
    inline bool approx_match(T actual, T expected, double max_rel_err) noexcept {
        if (std::isnan(expected)) return std::isnan(actual);
        if (std::isinf(expected)) return std::isinf(actual) && (std::signbit(actual) == std::signbit(expected));
        if (expected == T(0)) return actual == T(0) && (std::signbit(actual) == std::signbit(expected));
        const double diff = std::abs(static_cast<double>(actual) - static_cast<double>(expected));
        const double scale = std::abs(static_cast<double>(expected));
        return diff <= scale * max_rel_err;
    }

    template <class T>
    inline bool ref_is_nan(T x) noexcept {
        if constexpr (sizeof(T) == 8)
            return (raze::math::bit_cast<raze::u64>(x) & 0x7FFFFFFFFFFFFFFFull) > 0x7FF0000000000000ull;
        else
            return (raze::math::bit_cast<raze::u32>(x) & 0x7FFFFFFFu) > 0x7F800000u;
    }

    template <class T>
    inline bool ref_is_inf(T x) noexcept {
        if constexpr (sizeof(T) == 8)
            return (raze::math::bit_cast<raze::u64>(x) & 0x7FFFFFFFFFFFFFFFull) == 0x7FF0000000000000ull;
        else
            return (raze::math::bit_cast<raze::u32>(x) & 0x7FFFFFFFu) == 0x7F800000u;
    }

    template <class T>
    inline bool ref_is_finite(T x) noexcept {
        return !ref_is_inf(x) && !ref_is_nan(x);
    }

    template <class T>
    inline bool ref_is_zero(T x) noexcept {
        if constexpr (sizeof(T) == 8)
            return (raze::math::bit_cast<raze::u64>(x) & 0x7FFFFFFFFFFFFFFFull) == 0ull;
        else
            return (raze::math::bit_cast<raze::u32>(x) & 0x7FFFFFFFu) == 0u;
    }

    template <class T>
    inline bool ref_is_subnormal(T x) noexcept {
        if constexpr (sizeof(T) == 8) {
            const raze::u64 a = raze::math::bit_cast<raze::u64>(x) & 0x7FFFFFFFFFFFFFFFull;
            return a != 0ull && a < 0x0010000000000000ull;
        }
        else {
            const raze::u32 a = raze::math::bit_cast<raze::u32>(x) & 0x7FFFFFFFu;
            return a != 0u && a < 0x00800000u;
        }
    }

    template <class T>
    inline bool ref_is_normal(T x) noexcept {
        return ref_is_finite(x) && !ref_is_zero(x) && !ref_is_subnormal(x);
    }

    template <class T>
    inline bool ref_signbit(T x) noexcept {
        if constexpr (sizeof(T) == 8)
            return (raze::math::bit_cast<raze::u64>(x) >> 63) != 0ull;
        else
            return (raze::math::bit_cast<raze::u32>(x) >> 31) != 0u;
    }

    template <class V, class Op, class Ref>
    inline void test_unary_predicate(Op op, Ref ref, raze::u64 seed) {
        using T = typename V::value_type;
        using Mask = typename V::mask_type;
        constexpr size_t N = V::size();

        const auto specials = classify_values<T>();
        alignas(alignof(V)) T arr[N];

        auto check = [&](const Mask& m) {
            const V a = raze::vx::load<V>(arr);
            RTTS_ALL_VALIDATE_VECTOR(op(a), [&](size_t i) { return ref(arr[i]); });
            RTTS_ALL_VALIDATE_VECTOR(op[m](a), [&](size_t i) { return m[i] && ref(arr[i]); });
        };

        for (size_t shift = 0; shift < specials.size(); ++shift) {
            for (size_t i = 0; i < N; ++i) arr[i] = specials[(i + shift) % specials.size()];
            check(Mask(true));
            check(Mask(false));
            check(rtts::simd::make_alternating_mask<Mask>());
            check(rtts::simd::make_random_mask<Mask>());
        }

        std::mt19937_64 rng(seed + sizeof(T) * 19 + N);
        rtts::random::generator<T> gen(seed + sizeof(T) * 31 + N);

        for (size_t trial = 0; trial < 200; ++trial) {
            for (size_t i = 0; i < N; ++i)
                arr[i] = (rng() % 3 == 0) ? specials[rng() % specials.size()] : gen();
            check(rtts::simd::make_random_mask<Mask>());
        }
    }

    template <class V, class Op, class Ref>
    inline void test_unary_exact(Op op, Ref ref, const std::vector<typename V::value_type>& vals, raze::u64 seed) {
        using T = typename V::value_type;
        using Mask = typename V::mask_type;
        constexpr size_t N = V::size();

        alignas(alignof(V)) T arrA[N];
        alignas(alignof(V)) T arrSrc[N];

        auto check = [&](const Mask& m) {
            const V a = raze::vx::load<V>(arrA);
            const V src = raze::vx::load<V>(arrSrc);

            const V r1 = op(a);
            const V r2 = op[m](a);
            const V r3 = op[m, src](a);

            for (size_t i = 0; i < N; ++i) {
                const T e = ref(arrA[i]);
                RTTS_EXPECT(bits_match(r1[i], e));
                RTTS_EXPECT(bits_match(r2[i], m[i] ? e : T(0)));
                RTTS_EXPECT(bits_match(r3[i], m[i] ? e : arrSrc[i]));
            }
        };

        for (size_t shift = 0; shift < vals.size(); ++shift) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = vals[(i + shift) % vals.size()];
                arrSrc[i] = T(50 + i);
            }
            check(Mask(true));
            check(Mask(false));
            check(rtts::simd::make_alternating_mask<Mask>());
            check(rtts::simd::make_random_mask<Mask>());
        }

        std::mt19937_64 rng(seed + sizeof(T) * 29 + N);
        rtts::random::generator<T> gen(seed + sizeof(T) * 53 + N);

        for (size_t trial = 0; trial < 300; ++trial) {
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = (rng() % 4 == 0) ? vals[rng() % vals.size()] : gen();
                arrSrc[i] = gen();
            }
            check(rtts::simd::make_random_mask<Mask>());
        }
    }
}

