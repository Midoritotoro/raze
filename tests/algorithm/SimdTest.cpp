#include <raze/datapar/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>
#include <raze/algorithm/fill/Fill.h>
#include <string>
#include <vector>
#include <numeric>
#include <limits>
#include <algorithm>
#include <iostream>
#include <random>


template <typename _Simd_>
void mask_compress_any(
    const typename _Simd_::value_type*  a,
    const typename _Simd_::value_type*  src,
    typename _Simd_::value_type*        dst,
    typename _Simd_::mask_type          mask)
{
    constexpr auto N = _Simd_::size();
    int m = 0;

    for (int j = 0; j < N; ++j)
        if (!mask[j])
            dst[m++] = a[j];

    for (int i = m; i < N; ++i)
        dst[i] = src[i];
}

template <class Mask>
Mask make_alternating_mask() {
    Mask m;
    for (size_t i = 0; i < Mask::elements(); ++i)
        m[i] = (i % 2) == 0;
    return m;
}

template <class Mask>
Mask make_random_mask() {
    Mask m;
    std::mt19937_64 rng(0x123456789ABCDEFULL);
    for (size_t i = 0; i < Mask::elements(); ++i)
        m[i] = (rng() & 1) != 0;
    return m;
}

template <class T, size_t N, class Simd, class Mask,
    class WhereExpr, class WhereZeroExpr, class SimdOp, class ScalarOp>
void test_where_binary(
    const T(&arrA)[N], const T(&arrB)[N], const T(&arrSrc)[N],
    const Mask& m, const Simd& a, const Simd& b,
    const Simd& src, const WhereExpr& w,
    const WhereZeroExpr& wz,  SimdOp simd_op, ScalarOp scalar_op) noexcept
{
    {
        auto r1 = simd_op(w, b);
        auto r2 = simd_op(b, w);

        for (size_t i = 0; i < N; ++i) {
            if constexpr (raze::type_traits::is_any_of_v<std::remove_cvref_t<SimdOp>, raze::type_traits::bit_or<>,
                raze::type_traits::bit_xor<>, raze::type_traits::bit_and<>> && std::is_floating_point_v<T>)
            {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;

                auto expected1 = std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], arrB[i], arrSrc[i], m[i], false));
                auto expected2 = std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], arrB[i], arrSrc[i], m[i], true));
                T extracted1 = r1[i];
                T extracted2 = r2[i];
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted1) == expected1);
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted2) == expected2);
            }
            else {
                T expected1 = scalar_op(arrA[i], arrB[i], arrSrc[i], m[i], false);
                T expected2 = scalar_op(arrA[i], arrB[i], arrSrc[i], m[i], true);
                raze_assert(r1[i] == expected1);
                raze_assert(r2[i] == expected2);
            }
        }
    }

    {
        auto r1 = simd_op(wz, b); 
        auto r2 = simd_op(b, wz);

        for (size_t i = 0; i < N; ++i) {
            if constexpr (raze::type_traits::is_any_of_v<std::remove_cvref_t<SimdOp>, raze::type_traits::bit_or<>,
                raze::type_traits::bit_xor<>, raze::type_traits::bit_and<>> && std::is_floating_point_v<T>)
            {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;

                auto expected1 = m[i] ? std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], arrB[i], T(0), true, false)) : _Reinterpret_type(0);
                auto expected2 = m[i] ? std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], arrB[i], T(0), true, true)) : _Reinterpret_type(0);
                T extracted1 = r1[i];
                T extracted2 = r2[i];
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted1) == expected1);
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted2) == expected2);
            }
            else {
                T expected1 = m[i] ? scalar_op(arrA[i], arrB[i], T(0), true, false) : T(0);
                T expected2 = m[i] ? scalar_op(arrA[i], arrB[i], T(0), true, true) : T(0);
                raze_assert(r1[i] == expected1);
                raze_assert(r2[i] == expected2);
            }
        }
    }
}

template <class T, size_t N, class Simd, class Mask,
    class WhereExpr, class WhereZeroExpr, class SimdOp, class ScalarOp>
void test_where_unary(
    const T(&arrA)[N], const T(&arrSrc)[N],
    const Mask& m, const Simd& a,
    const Simd& src, WhereExpr& w,
    WhereZeroExpr& wz,  SimdOp simd_op, ScalarOp scalar_op) noexcept
{
    Simd a_copy = a;

    {
        auto r1 = simd_op(w);
        
        for (size_t i = 0; i < N; ++i) {
            if constexpr (raze::type_traits::is_any_of_v<std::remove_cvref_t<SimdOp>,
                raze::type_traits::bit_not<>> && std::is_floating_point_v<T>)
            {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;

                auto expected1 = std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], arrSrc[i], m[i], false));
                T extracted1 = r1[i];
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted1) == expected1);
            }
            else {
                T expected1 = scalar_op(arrA[i], arrSrc[i], m[i], false);
                raze_assert(r1[i] == expected1);
            }
        }
        w = a_copy;
    }

    {
        auto r1 = simd_op(wz); 

        for (size_t i = 0; i < N; ++i) {
            if constexpr (raze::type_traits::is_any_of_v<std::remove_cvref_t<SimdOp>,
                raze::type_traits::bit_not<>> && std::is_floating_point_v<T>)
            {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;
                auto expected1 = m[i] ? std::bit_cast<_Reinterpret_type>(scalar_op(arrA[i], T(0), true, false)) : _Reinterpret_type(0);
                T extracted1 = r1[i];
                raze_assert(std::bit_cast<_Reinterpret_type>(extracted1) == expected1);
            }
            else {
                T expected1 = m[i] ? scalar_op(arrA[i], T(0), true, false) : T(0);
                raze_assert(r1[i] == expected1);
            }
        }

        wz = a_copy;
    }
}

template <
    class           Simd,
    raze::uint64    Seed>
void test_shuffle_with_compile_time_pattern() {
    using T = typename Simd::value_type;
    constexpr std::size_t N = Simd::size();

    alignas(64) T arr[N];
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<T>(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    {
        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::datapar::shuffle(v, std::integer_sequence<raze::uint64, static_cast<raze::uint64>((I * Seed * 1103515245u + 12345u) % N)...>{});

            constexpr std::size_t indices[] = { I... };
            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[(indices[i] * Seed * 1103515245u + 12345u) % N]);
        }(std::make_index_sequence<N>{});
    }

    {
        constexpr auto make_idx = [](std::size_t i) constexpr {
            return (Seed * 6364136223846793005ULL + i * 1442695040888963407ULL) % N;
        };

        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::datapar::shuffle(v, std::integer_sequence<raze::uint64, static_cast<raze::uint64>(make_idx(I))...>{});

            constexpr std::size_t idxs[] = { make_idx(I)... };

            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[idxs[i]]);
        }(std::make_index_sequence<N>{});
    }

    {
        constexpr std::size_t a = (Seed * 1664525u + 1013904223u) % N;
        constexpr std::size_t b = (Seed * 22695477u + 1u) % N;

        constexpr auto make_idx = [](std::size_t i) constexpr {
            return (a * i + b) % N;
        };

        [&] <std::size_t... I>(std::index_sequence<I...>) {
            auto r = raze::datapar::shuffle(v, std::integer_sequence<raze::uint64, static_cast<raze::uint64>(make_idx(I))...>{});

            constexpr std::size_t idxs[] = { make_idx(I)... };

            for (std::size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[idxs[i]]);
        }(std::make_index_sequence<N>{});
    }
}

template <
    class               Simd,
    raze::uint64 ...    Seeds>
void test_shuffle_with_compile_time_all_patterns(std::index_sequence<Seeds...>) {
    (test_shuffle_with_compile_time_pattern<Simd, Seeds>(), ...);
}

template<class Simd, class Mask, class T, size_t N>
void test_simd_basics() {
    Simd v1;
    Simd v2(5);

    for (size_t i = 0; i < N; ++i)
        raze_assert(v2[i] == 5);

    alignas(64) T arr[N];
    std::iota(arr, arr + N, 1);

    Simd v3 = raze::datapar::load<Simd>(arr);
    for (size_t i = 0; i < N; ++i)
        raze_assert(v3[i] == arr[i]);

    Simd v4(v3);
    Simd v5 = v3;

    for (size_t i = 0; i < N; ++i) {
        raze_assert(v4[i] == arr[i]);
        raze_assert(v5[i] == arr[i]);
    }

    Simd v;
    v.fill(42);
    for (size_t i = 0; i < N; ++i)
        raze_assert(v[i] == 42);

    v[0] = 99;
    raze_assert(v[0] == 99);
}

template<class Simd, class Mask, class T, size_t N>
void test_load_store() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    const auto test_all = [&]<class _AlignmentPolicy_>(_AlignmentPolicy_ && policy) {
        Simd v = raze::datapar::load<Simd>(arr, policy);

        alignas(64) T out[N];
        raze::datapar::store(out, v, policy);

        for (size_t i = 0; i < N; ++i)
            raze_assert(out[i] == arr[i]);
    };

    test_all(raze::datapar::aligned_policy());
    test_all(raze::datapar::unaligned_policy());
}

template<class Simd, class Mask, class T, size_t N>
void test_masked_load_store() {
    Mask mask = make_alternating_mask<Mask>();
    alignas(64) T src[N], fallback[N], dst[N];

    for (size_t i = 0; i < N; ++i) {
        src[i] = T(i + 1);
        fallback[i] = T(100 + i);
        dst[i] = T(200 + i);
    }

    const auto test_all = [&]<class _AlignmentPolicy_>(_AlignmentPolicy_ && policy) {
        {
            Simd v = raze::datapar::load(raze::datapar::where(src, mask), policy);
            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (mask[i] ? src[i] : T(0)));
        }

        {
            Simd fb = raze::datapar::load<Simd>(fallback);
            Simd v = raze::datapar::load(raze::datapar::where(src, fb, mask), policy);

            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (mask[i] ? src[i] : fallback[i]));
        }

        {
            Simd val(77);
            raze::datapar::store(dst, raze::datapar::where(val, mask), policy);

            for (size_t i = 0; i < N; ++i)
                raze_assert(dst[i] == (mask[i] ? T(77) : T(200 + i)));
        }
    };

    test_all(raze::datapar::aligned_policy());
    test_all(raze::datapar::unaligned_policy());
}



template<class Simd, class Mask, class T, size_t N>
void test_where_semantics() {
    alignas(64) T arrA[N], arrB[N], arrSrc[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i] = i + 1;
        arrB[i] = (i + 1) * 3;
        arrSrc[i] = 100 + i;
    }

    Simd a = raze::datapar::load<Simd>(arrA);
    Simd b = raze::datapar::load<Simd>(arrB);
    Simd src = raze::datapar::load<Simd>(arrSrc);

    Mask m;
    for (size_t i = 0; i < N; ++i)
        m[i] = (i % 2 == 0);

    auto w = raze::datapar::where(a, src, m);
    auto wz = raze::datapar::where(a, m);

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        [](auto A) { return ++A; },
        [](T A, T Src, bool cond, bool rev) {
            return cond ? (++A) : Src;
    });

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        [](auto A) { return A++; },
        [](T A, T Src, bool cond, bool rev) {
            return cond ? (A++) : Src;
    });

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        [](auto A) { return --A; },
        [](T A, T Src, bool cond, bool rev) {
            return cond ? (--A) : Src;
    });

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        [](auto A) { return A--; },
        [](T A, T Src, bool cond, bool rev) {
            return cond ? (A--) : Src;
    });

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        raze::type_traits::negate{},
        [](T A, T Src, bool cond, bool rev) {
            return cond ? (-A) : Src;
    });

    test_where_unary<T, N>(
        arrA, arrSrc, m, a, src, w, wz,
        raze::type_traits::bit_not{},
        [](T A, T Src, bool cond, bool rev) {
            if constexpr (std::is_floating_point_v<T>) {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;
                return cond ? std::bit_cast<T>(~std::bit_cast<_Reinterpret_type>(A)) : Src;
            }
            else {
                return cond ? (~A) : Src;
            }
    });

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::plus{},
        [](T A, T B, T Src, bool cond, bool rev) {
            return cond ? (rev ? B + A : A + B) : Src;
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::minus{},
        [](T A, T B, T Src, bool cond, bool rev) {
            return cond ? (rev ? B - A : A - B) : Src;
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::multiplies{},
        [](T A, T B, T Src, bool cond, bool rev) {
            return cond ? (rev ? B * A : A * B) : Src;
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::divides{},
        [](T A, T B, T Src, bool cond, bool rev) {
            return cond ? (rev ? B / A : A / B) : Src;
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::bit_and{},
        [](T A, T B, T Src, bool cond, bool rev) {
            if constexpr (std::is_floating_point_v<T>) { 
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;
                return cond ? std::bit_cast<T>((std::bit_cast<_Reinterpret_type>(B)
                    & std::bit_cast<_Reinterpret_type>(A))) : Src;
            }
            else {
                return cond ? (A & B) : Src;
            }
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::bit_or{},
        [](T A, T B, T Src, bool cond, bool rev) {
            if constexpr (std::is_floating_point_v<T>) {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;
                return cond ? std::bit_cast<T>((std::bit_cast<_Reinterpret_type>(B)
                    | std::bit_cast<_Reinterpret_type>(A))) : Src;
            }
            else {
                return cond ? (A | B) : Src;
            }
        }
    );

    test_where_binary<T, N>(
        arrA, arrB, arrSrc, m,
        a, b, src, w, wz,
        raze::type_traits::bit_xor{},
        [](T A, T B, T Src, bool cond, bool rev) {
            if constexpr (std::is_floating_point_v<T>) {
                using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;
                return cond ? std::bit_cast<T>((std::bit_cast<_Reinterpret_type>(B)
                    ^ std::bit_cast<_Reinterpret_type>(A))) : Src;
            }
            else {
                return cond ? (A ^ B) : Src;
            }
        }
    );
}

template<class Simd, class Mask, class T, size_t N>
void test_shuffle_rotate_reverse() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    {
        auto r = raze::datapar::reverse(v);
        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[N - 1 - i]);
    }

    for (size_t sh = 0; sh < N; ++sh) {
        auto rl = raze::datapar::rotate_left(v, sh);
        auto rr = raze::datapar::rotate_right(v, sh);

        for (size_t i = 0; i < N; ++i) {
            raze_assert(rl[i] == arr[(i + sh) % N]);
            raze_assert(rr[i] == arr[(i + N - sh) % N]);
        }
    }

    test_shuffle_with_compile_time_all_patterns<Simd>(std::make_index_sequence<4>{});
}

template<class Simd, class Mask, class T, size_t N>
void test_slide_runtime() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    for (size_t sh = 0; sh <= N; ++sh) {
        auto r = raze::datapar::slide_left(v, sh);

        for (size_t i = 0; i < N; ++i) {
            T expected = (i + sh < N) ? arr[i + sh] : T(0);
            raze_assert(r[i] == expected);
        }
    }

    for (size_t sh = 0; sh <= N; ++sh) {
        auto r = raze::datapar::slide_right(v, sh);

        for (size_t i = 0; i < N; ++i) {
            T expected = (i >= sh) ? arr[i - sh] : T(0);
            raze_assert(r[i] == expected);
        }
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_slide_compiletime() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    [&] <size_t... I>(std::index_sequence<I...>) {
        (([&] {
            constexpr size_t sh = I;
            auto r = raze::datapar::slide_left(v, std::integral_constant<size_t, sh>{});

            for (size_t i = 0; i < N; ++i) {
                T expected = (i + sh < N) ? arr[i + sh] : T(0);
                raze_assert(r[i] == expected);
            }
            }()), ...);

        (([&] {
            constexpr size_t sh = I;
            auto r = raze::datapar::slide_right(v, std::integral_constant<size_t, sh>{});

            for (size_t i = 0; i < N; ++i) {
                T expected = (i >= sh) ? arr[i - sh] : T(0);
                raze_assert(r[i] == expected);
            }
            }()), ...);

    }(std::make_index_sequence<N + 1>{});
}

template<class Simd, class Mask, class T, size_t N>
void test_rotate_runtime() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    for (size_t sh = 0; sh < N; ++sh) {
        auto r = raze::datapar::rotate_left(v, sh);

        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[(i + sh) % N]);
    }

    for (size_t sh = 0; sh < N; ++sh) {
        auto r = raze::datapar::rotate_right(v, sh);

        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[(i + N - sh) % N]);
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_rotate_compiletime() {
    alignas(64) T arr[N];
    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    [&] <size_t... I>(std::index_sequence<I...>) {
        (([&] {
            constexpr size_t sh = I;
            auto r = raze::datapar::rotate_left(v, std::integral_constant<size_t, sh>{});

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[(i + sh) % N]);
            }()), ...);

        (([&] {
            constexpr size_t sh = I;
            auto r = raze::datapar::rotate_right(v, std::integral_constant<size_t, sh>{});

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[(i + N - sh) % N]);
            }()), ...);

    }(std::make_index_sequence<N>{});
}


template<class Simd, class Mask, class T, size_t N>
void test_compress() {
    alignas(64) T src[N];
    for (size_t i = 0; i < N; ++i)
        src[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(src);
    Mask mask = make_random_mask<Mask>();

    alignas(64) T dst[N], expected[N];
    mask_compress_any<Simd>(src, src, expected, mask);

    raze::datapar::compress_store(dst, v, mask);
    raze_assert(std::equal(dst, dst + N, expected, expected + N));
}

template<class Simd, class Mask, class T, size_t N>
void test_arithmetic() {
    alignas(64) T arrA[N], arrB[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T((i + 1) * 3);
    }

    Simd a = raze::datapar::load<Simd>(arrA);
    Simd b = raze::datapar::load<Simd>(arrB);

    {
        auto v = a + b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] + arrB[i]));
    }

    {
        auto v = a - b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] - arrB[i]));
    }

    {
        auto v = a * b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] * arrB[i]));
    }

    {
        for (size_t i = 0; i < N; ++i)
            arrB[i] = T(i + 1);

        b = raze::datapar::load<Simd>(arrB);
        auto v = a / b;

        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] / arrB[i]));
    }

    {
        auto v = -a;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(-arrA[i]));
    }

    {
        Simd v = a;
        v += b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] + arrB[i]));
    }

    {
        Simd v = a;
        v -= b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] - arrB[i]));
    }

    {
        Simd v = a;
        v *= b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] * arrB[i]));
    }

    {
        Simd v = a;
        v /= b;
        for (size_t i = 0; i < N; ++i)
            raze_assert(v[i] == T(arrA[i] / arrB[i]));
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_comparisons() {
    alignas(64) T arrA[N], arrB[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T(i + 1);
        arrB[i] = T(N - i);
    }

    Simd a = raze::datapar::load<Simd>(arrA);
    Simd b = raze::datapar::load<Simd>(arrB);

    {
        auto m = (a == a);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == true);
    }

    {
        auto m = (a != b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (arrA[i] != arrB[i]));
    }

    {
        auto m = (a < b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (arrA[i] < arrB[i]));
    }

    {
        auto m = (a > b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (arrA[i] > arrB[i]));
    }

    {
        auto m = (a <= b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (arrA[i] <= arrB[i]));
    }

    {
        auto m = (a >= b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (arrA[i] >= arrB[i]));
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_bitwise() {
    if constexpr (std::is_integral_v<T>) {
        alignas(64) T arrA[N], arrB[N];

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = T((i * 0x5A5A) ^ (i + 3));
            arrB[i] = T((N - i) * 0x33 ^ (i * 7));
        }

        arrA[N - 1] = raze::math::__maximum_integral_limit<T>();
        arrA[0]     = raze::math::__minimum_integral_limit<T>();

        Simd a = raze::datapar::load<Simd>(arrA);
        Simd b = raze::datapar::load<Simd>(arrB);

        {
            auto v = a & b;
            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] & arrB[i]));
        }

        {
            auto v = a | b;
            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] | arrB[i]));
        }

        {
            auto v = a ^ b;
            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] ^ arrB[i]));
        }

        {
            auto v = ~a;
            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == T(~arrA[i]));
        }

        {
            auto v = (a & b) ^ (~a | b);
            for (size_t i = 0; i < N; ++i) {
                T expected = (arrA[i] & arrB[i]) ^ (~arrA[i] | arrB[i]);
                raze_assert(v[i] == expected);
            }
        }

        {
            Simd v = a;
            v &= b;

            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] & arrB[i]));
        }

        {
            Simd v = a;
            v |= b;

            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] | arrB[i]));
        }

        {
            Simd v = a;
            v ^= b;

            for (size_t i = 0; i < N; ++i)
                raze_assert(v[i] == (arrA[i] ^ arrB[i]));
        }

        for (raze::uint32 sh = 0; sh <= sizeof(T) * 8; ++sh) {
            {
                Simd v = a;
                v <<= sh;

                for (size_t i = 0; i < N; ++i) {
                    T expected = 0;
                    if (sh < sizeof(T) * 8)
                        expected = T(arrA[i] << sh);

                    raze_assert(v[i] == expected);
                }
            }

            {
                Simd v = a;
                v >>= sh;

                for (size_t i = 0; i < N; ++i) {
                    T expected = 0;

                    if (sh < sizeof(T) * 8) {
                        expected = T(arrA[i] >> sh);
                    } else {
                        if constexpr (std::is_signed_v<T>)
                            expected = T(arrA[i] >> (sizeof(T) * 8 - 1));
                    }

                    raze_assert(v[i] == expected);
                }
            }
        }
    }
}
template<class Simd, class Mask, class T, size_t N>
void test_select()
{
    alignas(64) T a_arr[N], b_arr[N];
    for (size_t i = 0; i < N; ++i) {
        a_arr[i] = T(i + 1);
        b_arr[i] = T(200 + i);
    }

    Simd a = raze::datapar::load<Simd>(a_arr);
    Simd b = raze::datapar::load<Simd>(b_arr);

    auto test_mask = [&](Mask mask) {
        auto r = raze::datapar::select(a, b, mask);

        for (size_t i = 0; i < N; ++i) {
            T expected = mask[i] ? a_arr[i] : b_arr[i];
            raze_assert(r[i] == expected);
        }
    };

    test_mask(Mask(true));
    test_mask(Mask(false));
    test_mask(make_alternating_mask<Mask>());
    test_mask(make_random_mask<Mask>());
}

template<class Simd, class Mask, class T, size_t N>
void test_reduce_add() {
    alignas(64) T arr[N];

    for (size_t i = 0; i < N; ++i)
        arr[i] = T(i + 1);

    Simd v = raze::datapar::load<Simd>(arr);

    {
        auto r = raze::datapar::reduce_add(v);

        raze::datapar::__reduce_type<T> expected = 0;
        for (size_t i = 0; i < N; ++i)
            expected += arr[i];

        raze_assert(r == expected);
    }

    {
        Mask m = make_alternating_mask<Mask>();

        Simd a = v;
        auto w = raze::datapar::where(a, m);

        auto r = raze::datapar::reduce_add(w);

        raze::datapar::__reduce_type<T> expected = 0;
        for (size_t i = 0; i < N; ++i)
            if (m[i])
                expected += arr[i];

        raze_assert(r == expected);
    }

    {
        Mask m = make_alternating_mask<Mask>();

        Simd a = v;
        auto wz = raze::datapar::where(a, m);

        auto r = raze::datapar::reduce_add(wz);

        raze::datapar::__reduce_type<T> expected = 0;
        for (size_t i = 0; i < N; ++i)
            if (m[i])
                expected += arr[i];

        raze_assert(r == expected);
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_abs() {
    alignas(64) T arr[N];
    alignas(64) T fallback[N];

    std::iota(fallback, fallback + N, 1);

    for (size_t i = 0; i < N; ++i) {
        if constexpr (std::is_signed_v<T>)
            arr[i] = T((i % 2 == 0) ? -(T(i) + 1) : (T(i) + 1));
        else
            arr[i] = T(i + 1);
    }
    
    Simd v = raze::datapar::load<Simd>(arr);
    Simd fbk = raze::datapar::load<Simd>(fallback);

    {
        auto r = raze::datapar::abs(v);

        for (size_t i = 0; i < N; ++i) {
            T expected = arr[i] < T(0) ? T(-arr[i]) : arr[i];
            raze_assert(r[i] == expected);
        }
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto w = raze::datapar::where(v, fbk, m);

        auto r = raze::datapar::abs(w);

        for (size_t i = 0; i < N; ++i) {
            T expected = m[i] ? raze::math::abs(arr[i]) : fallback[i];
            raze_assert(r[i] == expected);
        }
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto wz = raze::datapar::where(v, m);

        auto r = raze::datapar::abs(wz);

        for (size_t i = 0; i < N; ++i) {
            T expected = m[i] ? raze::math::abs(arr[i]) : 0;
            raze_assert(r[i] == expected);
        }
    }
}

template<class Simd, class Mask, class T, size_t N>
void test_horizontal_min_max() {
    alignas(64) T arr[N];
    alignas(64) T fallback[N];

    std::iota(fallback, fallback + N, 1);

    for (size_t i = 0; i < N; ++i)
        arr[i] = T((i % 3 == 0) ? -(i + 5) : (i + 1));

    Simd v = raze::datapar::load<Simd>(arr);
    Simd fbk = raze::datapar::load<Simd>(fallback);

    {
        auto rmin = raze::datapar::horizontal_min(v);
        auto rmax = raze::datapar::horizontal_max(v);

        T expected_min = arr[0];
        T expected_max = arr[0];

        for (size_t i = 1; i < N; ++i) {
            expected_min = std::min(expected_min, arr[i]);
            expected_max = std::max(expected_max, arr[i]);
        }

        raze_assert(rmin == expected_min);
        raze_assert(rmax == expected_max);
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto w = raze::datapar::where(v, fbk, m);

        auto rmin = raze::datapar::horizontal_min(w);
        auto rmax = raze::datapar::horizontal_max(w);

        T expected_min = m[0] ? arr[0] : fallback[0];
        T expected_max = m[0] ? arr[0] : fallback[0];

        for (size_t i = 1; i < N; ++i) {
            expected_min = m[i] ? std::min(expected_min, arr[i]) : std::min(expected_min, fallback[i]);
            expected_max = m[i] ? std::max(expected_max, arr[i]) : std::max(expected_max, fallback[i]);
        }

        raze_assert(rmin == expected_min);
        raze_assert(rmax == expected_max);
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto wz = raze::datapar::where(v, m);

        auto rmin = raze::datapar::horizontal_min(wz);
        auto rmax = raze::datapar::horizontal_max(wz);

        T expected_min = m[0] ? arr[0] : T(0);
        T expected_max = expected_min;

        for (size_t i = 1; i < N; ++i) {
            T val = m[i] ? arr[i] : T(0);
            expected_min = std::min(expected_min, val);
            expected_max = std::max(expected_max, val);
        }

        raze_assert(rmin == expected_min);
        raze_assert(rmax == expected_max);
    }
}


template<class Simd, class Mask, class T, size_t N>
void test_vertical_min_max() {
    alignas(64) T arrA[N], arrB[N];
    alignas(64) T fallback[N];

    std::iota(fallback, fallback + N, 1);


    for (size_t i = 0; i < N; ++i) {
        arrA[i] = T((i % 2 == 0) ? (i + 1) : -(i + 3));
        arrB[i] = T((i % 3 == 0) ? (i + 5) : -(i + 2));
    }

    Simd a = raze::datapar::load<Simd>(arrA);
    Simd b = raze::datapar::load<Simd>(arrB);
    Simd fbk = raze::datapar::load<Simd>(fallback);

    {
        auto vmin = raze::datapar::vertical_min(a, b);
        auto vmax = raze::datapar::vertical_max(a, b);

        for (size_t i = 0; i < N; ++i) {
            T emin = std::min(arrA[i], arrB[i]);
            T emax = std::max(arrA[i], arrB[i]);

            raze_assert(vmin[i] == emin);
            raze_assert(vmax[i] == emax);
        }
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto wz = raze::datapar::where(a, m);

        auto wmin = raze::datapar::vertical_min(b, wz);
        auto wmax = raze::datapar::vertical_max(b, wz);

        for (size_t i = 0; i < N; ++i) {
            if (m[i]) {
                T emin = std::min(arrB[i], arrA[i]);
                T emax = std::max(arrB[i], arrA[i]);
                raze_assert(wmin[i] == emin || wmin[i] == emax);
            }
            else {
                raze_assert(wmin[i] == T(0));
            }
        }
    }

    {
        Mask m = make_alternating_mask<Mask>();
        auto w = raze::datapar::where(a, fbk, m);

        auto wmin = raze::datapar::vertical_min(w, b);
        auto wmax = raze::datapar::vertical_max(w, b);

        for (size_t i = 0; i < N; ++i) {
            if (m[i]) {
                T emin = std::min(arrA[i], arrB[i]);
                T emax = std::max(arrA[i], arrB[i]);
                raze_assert(wmin[i] == emin || wmin[i] == emax);
            }
            else {
                raze_assert(wmin[i] == fallback[i]);
            }
        }
    }
}

template <typename T, raze::arch::ISA Arch,raze::uint32 Width>
void test_methods() {
    using Simd = raze::datapar::simd<T, raze::datapar::x86_abi<Arch, Width>>;
    using Mask = typename Simd::mask_type;
    constexpr size_t N = Simd::size();

    if (!raze::arch::ProcessorFeatures::isSupported<Simd::__isa>()) return;

    //test_simd_basics<Simd, Mask, T, N>();
    //test_load_store<Simd, Mask, T, N>();
    //test_masked_load_store<Simd, Mask, T, N>();
    //test_comparisons<Simd, Mask, T, N>();
    //test_arithmetic<Simd, Mask, T, N>();
    //test_bitwise<Simd, Mask, T, N>();
    //test_reduce_add<Simd, Mask, T, N>();
    //test_abs<Simd, Mask, T, N>();
    //test_horizontal_min_max<Simd, Mask, T, N>();
    //test_vertical_min_max<Simd, Mask, T, N>();
    test_where_semantics<Simd, Mask, T, N>();
    //test_shuffle_rotate_reverse<Simd, Mask, T, N>();
    //test_compress<Simd, Mask, T, N>();
    //test_slide_runtime<Simd, Mask, T, N>();
    //test_slide_compiletime<Simd, Mask, T, N>();
    //test_rotate_runtime<Simd, Mask, T, N>();
    //test_rotate_compiletime<Simd, Mask, T, N>();
    //test_select<Simd, Mask, T, N>();
}


template <raze::arch::ISA _ISA_, raze::uint32 _Width_>
void test_methods() {
    test_methods<raze::int8, _ISA_, _Width_>();
    test_methods<raze::uint8, _ISA_, _Width_>();

    test_methods<raze::int16, _ISA_, _Width_>();
    test_methods<raze::uint16, _ISA_, _Width_>();

    test_methods<raze::int32, _ISA_, _Width_>();
    test_methods<raze::uint32, _ISA_, _Width_>();

    test_methods<raze::int64, _ISA_, _Width_>();
    test_methods<raze::uint64, _ISA_, _Width_>();

    test_methods<float, _ISA_, _Width_>();
    test_methods<double, _ISA_, _Width_>();
}

int main() {
    test_methods<raze::arch::ISA::SSE2, 128>();
    test_methods<raze::arch::ISA::SSE3, 128>();
    test_methods<raze::arch::ISA::SSSE3, 128>();
    test_methods<raze::arch::ISA::SSE41, 128>();
    test_methods<raze::arch::ISA::SSE42, 128>();

    test_methods<raze::arch::ISA::AVX2, 128>();
    test_methods<raze::arch::ISA::AVX2, 256>();
    
    test_methods<raze::arch::ISA::AVX512F, 512>();
    test_methods<raze::arch::ISA::AVX512BW, 512>();
    test_methods<raze::arch::ISA::AVX512DQ, 512>();
    test_methods<raze::arch::ISA::AVX512BWDQ, 512>();
    test_methods<raze::arch::ISA::AVX512VBMI, 512>();
    test_methods<raze::arch::ISA::AVX512VBMI2, 512>();
    test_methods<raze::arch::ISA::AVX512VBMIDQ, 512>();
    test_methods<raze::arch::ISA::AVX512VBMI2DQ, 512>();

    test_methods<raze::arch::ISA::AVX512VLF, 128>();
    test_methods<raze::arch::ISA::AVX512VLBW, 128>();
    test_methods<raze::arch::ISA::AVX512VLBWDQ, 128>();
    test_methods<raze::arch::ISA::AVX512VLDQ, 128>();

    test_methods<raze::arch::ISA::AVX512VLF, 256>();
    test_methods<raze::arch::ISA::AVX512VLBW, 256>();
    test_methods<raze::arch::ISA::AVX512VLBWDQ, 256>();
    test_methods<raze::arch::ISA::AVX512VLDQ, 256>();

    test_methods<raze::arch::ISA::AVX512VBMIVL, 128>();
    test_methods<raze::arch::ISA::AVX512VBMI2VL, 128>();
    test_methods<raze::arch::ISA::AVX512VBMIVLDQ, 128>();
    test_methods<raze::arch::ISA::AVX512VBMI2VLDQ, 128>();

    test_methods<raze::arch::ISA::AVX512VBMIVL, 256>();
    test_methods<raze::arch::ISA::AVX512VBMI2VL, 256>();
    test_methods<raze::arch::ISA::AVX512VBMIVLDQ, 256>();
    test_methods<raze::arch::ISA::AVX512VBMI2VLDQ, 256>();

    return 0;
}
