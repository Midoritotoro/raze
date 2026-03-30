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

template <typename T, raze::arch::ISA Arch, raze::uint32 _Width_>
void test_methods() {
    using Simd = raze::datapar::simd<T, raze::datapar::x86_abi<Arch, _Width_>>;
    using Mask = typename Simd::mask_type;
    constexpr size_t N = Simd::size();

    if (raze::arch::ProcessorFeatures::isSupported<Simd::__isa>() == false) {
        std::cout << "Not supported" << '\n';
        return;
    }

    //{
    //    Simd v1;
    //    Simd v2(5);

    //    for (int i = 0; i < v2.size(); ++i)
    //        raze_assert(v2[i] == 5);

    //    alignas(16) T arr[N];
    //    std::iota(arr, arr + N, 1);

    //    Simd v3 = raze::datapar::load<Simd>(arr);
    //    for (int i = 0; i < v2.size(); ++i)
    //        raze_assert(v3[i] == arr[i]);

    //    Simd v4(v3);
    //    for (int i = 0; i < v2.size(); ++i) 
    //        raze_assert(v4[i] == arr[i]);

    //    Simd v5(v3);
    //    for (int i = 0; i < v2.size(); ++i)
    //        raze_assert(v5[i] == arr[i]);
    //}

    //{
    //    Simd v;
    //    v.fill(42);
    //    for (int i = 0; i < v.size(); ++i) 
    //        raze_assert(v[i] == 42);

    //    v[0] = 99;
    //    raze_assert(v[0] == 99);
    //}

    //{
    //    using raze::datapar::simd_cast;
    //    Simd v(11);

    //    auto vOther = simd_cast<raze::datapar::simd128_sse2<float>>(v);
    //    auto vOther2 = simd_cast<raze::arch::ISA::SSE2, float>(v);
    //    auto vOther3 = simd_cast<raze::arch::ISA::SSE2>(v);
    //    auto vOther4 = simd_cast<__m128i>(v);
    //    auto vOther5 = simd_cast<int>(v);

    //    static_assert(std::is_same_v<decltype(vOther), decltype(vOther2)>);
    //    static_assert(std::is_same_v<decltype(vOther2), raze::datapar::simd128_sse2<float>>);
    //    static_assert(std::is_same_v<decltype(vOther3), raze::datapar::simd<typename Simd::value_type, raze::datapar::x86_abi<raze::arch::ISA::SSE2, 128>>>);
    //    static_assert(std::is_same_v<decltype(vOther4), __m128i>);
    //    static_assert(std::is_same_v<decltype(vOther5), raze::datapar::simd<int, typename Simd::abi_type>>);
    //}

    //{
    //    const auto tests_for_mask = [](Mask mask) {
    //        alignas(64) T src[N];
    //        alignas(64) T dst[N];

    //        for (size_t i = 0; i < N; ++i)
    //            src[i] = static_cast<T>(i + 1);

    //        for (size_t i = 0; i < N; ++i)
    //            dst[i] = static_cast<T>(100 + i);

    //        Simd loaded_unaligned = raze::datapar::maskz_load<Simd>(src, mask);

    //        for (size_t i = 0; i < N; ++i) {
    //            if (mask[i])
    //                raze_assert(loaded_unaligned[i] == src[i]);
    //            else
    //                raze_assert(loaded_unaligned[i] == T(0));
    //        }

    //        Simd loaded_aligned = raze::datapar::maskz_load<Simd>(src, mask, raze::datapar::aligned_policy{});
    //        for (size_t i = 0; i < N; ++i) {
    //            if (mask[i])
    //                raze_assert(loaded_aligned[i] == src[i]);
    //            else
    //                raze_assert(loaded_aligned[i] == T(0));
    //        }

    //        Simd v(77);
    //        raze::datapar::mask_store(dst, v, mask);

    //        for (size_t i = 0; i < N; ++i) {
    //            if (mask[i])
    //                raze_assert(dst[i] == T(77));
    //            else
    //                raze_assert(dst[i] == T(100 + i));
    //        }

    //        for (size_t i = 0; i < N; ++i)
    //            dst[i] = static_cast<T>(200 + i);

    //        raze::datapar::mask_store(dst, v, mask, raze::datapar::aligned_policy{});
    //        for (size_t i = 0; i < N; ++i) {
    //            if (mask[i])
    //                raze_assert(dst[i] == T(77));
    //            else
    //                raze_assert(dst[i] == T(200 + i));
    //        }
    //    };

    //    bool maskArray[N];

    //    for (int i = 0; i < N; ++i)
    //        maskArray[i] = ((i % 2) == 0);

    //    tests_for_mask(true);
    //    tests_for_mask(false);
    //    tests_for_mask(maskArray);
    //}

    //{
    //    alignas(64) T src[N];
    //    for (size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i + 1);

    //    Simd v = raze::datapar::load<Simd>(src);

    //    Mask mask = 0;

    //    {
    //        alignas(64) T dst[N] = {};
    //        raze::datapar::compress_store(dst, v, mask, raze::datapar::aligned_policy{});

    //        alignas(64) T expected[N];
    //        mask_compress_any<Simd>(src, src, expected, mask);

    //        raze_assert(std::equal(expected, expected + N, dst));
    //    }

    //    {
    //        alignas(64) T dst[N] = {};
    //        raze::datapar::compress_store(dst, v, mask);

    //        alignas(64) T expected[N];
    //        mask_compress_any<Simd>(src, src, expected, mask);

    //        raze_assert(std::equal(expected, expected + N, dst));
    //    }

    //    {
    //        alignas(64) T dst[N] = {};
    //        raze::datapar::compress_store(dst, v, mask, raze::datapar::aligned_policy{});

    //        alignas(64) T expected[N];
    //        mask_compress_any<Simd>(src, src, expected, mask);

    //        raze_assert(std::equal(expected, expected + N, dst));
    //    }
    //}

    //{
    //    std::vector<T> va(N), vb(N), vc(N);

    //    for (size_t i = 0; i < N; ++i) {
    //        va[i] = static_cast<T>(i + 1);
    //        vb[i] = static_cast<T>(i + 1);
    //        vc[i] = static_cast<T>(i + 2);
    //    }

    //    Simd a = raze::datapar::load<Simd>(va.data());
    //    Simd b = raze::datapar::load<Simd>(vb.data());
    //    Simd c = raze::datapar::load<Simd>(vc.data());

    //    raze::datapar::simd<int, raze::datapar::x86_abi<raze::arch::ISA::SSE2, 128>> v1;
    //    raze::datapar::simd<int, raze::datapar::x86_abi<raze::arch::ISA::SSE2, 128>> v2;

    //    auto m = v1 == v2;

    //    raze_assert(raze::datapar::all_of(m));
    //    raze_assert(raze::datapar::any_of(m));

    //    auto mEq = (a == b);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mEq[i] == true);
    //    }

    //    auto mNeq = (a != c);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mNeq[i] == true);
    //    }

    //    auto mGt = (c > a);
    //    auto mLt = (a < c);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mGt[i] == true);
    //        raze_assert(mLt[i] == true);
    //    }

    //    auto mGe = (a >= b);
    //    auto mLe = (a <= b);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mGe[i] == true);
    //        raze_assert(mLe[i] == true);
    //    }
    //}

    //{
    //    alignas(64) T arr0[N], arrMax[N];
    //    for (size_t i = 0; i < N; ++i) {
    //        arr0[i] = 0;
    //        arrMax[i] = std::numeric_limits<T>::max();
    //    }

    //    Simd v0 = raze::datapar::load<Simd>(arr0);
    //    Simd vmax = raze::datapar::load<Simd>(arrMax);

    //    raze_assert(raze::datapar::any_of(v0 != vmax));

    //    auto mEq = (v0 == v0);
    //    auto mNeq = (v0 != vmax);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mEq[i] == true);
    //        raze_assert(mNeq[i] == true);
    //    }

    //    auto mLt = (v0 < vmax);
    //    auto mGt = (vmax > v0);
    //    auto mLe = (v0 <= v0);
    //    auto mGe = (vmax >= vmax);

    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(mLt[i] == true);
    //        raze_assert(mGt[i] == true);
    //        raze_assert(mLe[i] == true);
    //        raze_assert(mGe[i] == true);
    //    }
    //}

    //{
    //    for (long long step = 1; step < (1LL << (std::numeric_limits<T>::digits - 2)); step <<= 1) {
    //        alignas(64) T arrA[N], arrB[N];

    //        for (size_t i = 0; i < N; ++i) {
    //            arrA[i] = step;
    //            arrB[i] = step + 1;
    //        }
    //        Simd vA = raze::datapar::load<Simd>(arrA);
    //        Simd vB = raze::datapar::load<Simd>(arrB);

    //        auto mEq = (vA == vA);
    //        auto mNeq = (vA != vB);
    //        auto mLt = (vA < vB);
    //        auto mGt = (vB > vA);
    //        auto mLe = (vA <= vA);
    //        auto mGe = (vB >= vB);

    //        for (size_t i = 0; i < N; ++i) {
    //            raze_assert(mEq[i] == true);
    //            raze_assert(mNeq[i] == true);
    //            raze_assert(mLt[i] == true);
    //            raze_assert(mGt[i] == true);
    //            raze_assert(mLe[i] == true);
    //            raze_assert(mGe[i] == true);
    //        }
    //    }
    //}

    //{
    //    raze::datapar::__reduce_type<T> reduced = 0;

    //    alignas(64) T array[N] = {};
    //    for (auto i = 0; i < N; ++i) {
    //        array[i] = (unsigned char)(i * 0x7fdbu);
    //    }

    //    for (auto i = 0; i < N; ++i) {
    //        reduced += (unsigned char)(array[i]);
    //    }

    //    auto simdReduced = raze::datapar::reduce_add(raze::datapar::load<Simd>(array));

    //    raze_assert(simdReduced == reduced);
    //}

    //{
    //    alignas(64) T arrA[N], arrB[N];

    //    for (size_t i = 0; i < N; ++i) {
    //        if (i % 4 == 0)
    //            arrA[i] = static_cast<T>(100);
    //        else if (i % 5 == 0)
    //            arrA[i] = static_cast<T>(-50);
    //        else
    //            arrA[i] = static_cast<T>(i - 2);

    //        if (i % 3 == 0)
    //            arrB[i] = static_cast<T>(200);
    //        else if (i % 7 == 0)
    //            arrB[i] = static_cast<T>(-100);
    //        else
    //            arrB[i] = static_cast<T>(N - i);
    //    }

    //    Simd a = raze::datapar::load<Simd>(arrA);
    //    Simd b = raze::datapar::load<Simd>(arrB);

    //    auto absVec = raze::datapar::abs(a);
    //    for (size_t i = 0; i < N; ++i) {
    //        raze_assert(absVec[i] == static_cast<T>(raze::math::abs(arrA[i])));
    //    }
    //}

    //{
    //    alignas(64) T arr[N];
    //    alignas(64) T reversed[N];

    //    std::iota(arr, arr + N, 0);

    //    auto v = raze::datapar::reverse(raze::datapar::load<Simd>(arr));
    //    raze::datapar::store(reversed, v);

    //    for (size_t i = 0; i < N; ++i)
    //        raze_assert(reversed[i] == arr[N - 1 - i]);
    //}

    //for (size_t i = 0; i < N; ++i) {
    //    Simd v1(0);
    //    Simd v2(0);

    //    v1[i] = 42;
    //    v2[i] = 42;

    //    auto mask = (v1 == v2);

    //    raze_assert(raze::datapar::any_of(mask));
    //    raze_assert(raze::datapar::all_of(mask));

    //    raze_assert(raze::datapar::count_set(mask) == N);
    //    raze_assert(raze::datapar::find_first_set(mask) == 0);
    //    raze_assert(raze::datapar::find_last_set(mask) == 0);
    //}

    //{
    //    alignas(64) T arrA[N], arrB[N];

    //    for (size_t i = 0; i < N; ++i) {
    //        arrA[i] = static_cast<T>(i + 1);
    //        arrB[i] = static_cast<T>(N - i);
    //    }

    //    Simd a = raze::datapar::load<Simd>(arrA);
    //    Simd b = raze::datapar::load<Simd>(arrB);

    //    {
    //        auto v = a + b;
    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == arrA[i] + arrB[i]);
    //    }

    //    {
    //        auto v = a - b;
    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] - arrB[i]));
    //    }

    //    {
    //        auto v = a * b;
    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] * arrB[i]));
    //    }

    //    {
    //        for (size_t i = 0; i < N; ++i)
    //            arrB[i] = static_cast<T>(i + 1);

    //        b = raze::datapar::load<Simd>(arrB);

    //        auto v = a / b;
    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] / arrB[i]));

    //        for (size_t i = 0; i < N; ++i) {
    //            auto v2 = a / b[i];
    //            auto scalar = static_cast<T>(arrA[i] / arrB[i]);
    //            raze_assert(v2[i] == scalar);
    //        }
    //    }

    //    {
    //        auto a1 = (a + b);
    //        auto a2 = (a - b);
    //        auto v = a1 * a2;
    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(static_cast<T>(arrA[i] + arrB[i]) * static_cast<T>(arrA[i] - arrB[i])));
    //    }

    //    {
    //        auto v = -a;
    //        for (size_t i = 0; i < N; ++i) {
    //            auto t = static_cast<T>(-arrA[i]);
    //            raze_assert(v[i] == t);
    //        }
    //    }
    //}

    //{
    //    alignas(64) T arrA[N], arrB[N];

    //    for (size_t i = 0; i < N; ++i) {
    //        arrA[i] = static_cast<T>((i * 0x5A5A) ^ (i + 3));
    //        arrB[i] = static_cast<T>((N - i) * 0x33 ^ (i * 7));
    //    }

    //    Simd a = raze::datapar::load<Simd>(arrA);
    //    Simd b = raze::datapar::load<Simd>(arrB);

    //    if constexpr (std::is_integral_v<T>) {
    //        {
    //            auto v = a & b;
    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] & arrB[i]));
    //        }

    //        {
    //            auto v = a | b;
    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] | arrB[i]));
    //        }

    //        {
    //            auto v = a ^ b;
    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] ^ arrB[i]));
    //        }

    //        {
    //            auto v = ~a;
    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == static_cast<T>(~arrA[i]));
    //        }

    //        {
    //            auto v = (a & b) ^ (~a | b);
    //            for (size_t i = 0; i < N; ++i) {
    //                T val = ((arrA[i] & arrB[i]) ^ (~arrA[i] | arrB[i]));
    //                raze_assert(v[i] == val);
    //            }
    //        }
    //    }
    //}

    //{
    //    if constexpr (std::is_integral_v<T>) {
    //        alignas(64) T arrA[N];
    //        for (size_t i = 0; i < N; ++i)
    //            arrA[i] = static_cast<T>((i + 1) * 3);

    //        arrA[N - 1] = raze::math::__maximum_integral_limit<T>();
    //        arrA[0] = raze::math::__minimum_integral_limit<T>();

    //        Simd a = raze::datapar::load<Simd>(arrA);

    //        for (raze::uint32 sh = 0; sh <= (sizeof(T) * 8); ++sh) {
    //            {
    //                auto v = (a << sh);

    //                for (size_t i = 0; i < N; ++i) {
    //                    T expected = 0;

    //                    if (sh < (sizeof(T) * 8)) {
    //                        expected = static_cast<T>(arrA[i] << sh);
    //                    }
    //                    
    //                    raze_assert(v[i] == expected);
    //                }
    //            }

    //            {
    //                auto v = (a >> sh);

    //                for (size_t i = 0; i < N; ++i) {
    //                    T expected = 0;

    //                    if (sh < (sizeof(T) * 8)) {
    //                        expected = static_cast<T>(arrA[i] >> sh);
    //                    }
    //                    else {
    //                        if constexpr (std::is_signed_v<T>)
    //                            expected = arrA[i] >> (sizeof(T) * 8 - 1);
    //                    }

    //                    raze_assert(v[i] == expected);
    //                }
    //            }
    //        }
    //    }
    //}

    //{
    //    alignas(64) T arr[N];
    //    for (size_t i = 0; i < N; ++i)
    //        arr[i] = static_cast<T>(i + 1);

    //    Simd v = raze::datapar::load<Simd>(arr);

    //    for (raze::uint32 sh = 0; sh <= N; ++sh) {
    //        auto slid = raze::datapar::slide_left(v, sh);

    //        alignas(64) T expected[N];

    //        if (sh >= N) {
    //            for (size_t i = 0; i < N; ++i)
    //                expected[i] = T(0);
    //        }
    //        else {
    //            for (size_t i = 0; i < N; ++i) {
    //                if (i >= sh)
    //                    expected[i] = arr[i - sh];
    //                else
    //                    expected[i] = T(0);
    //            }
    //        }

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(slid[i] == expected[i]);
    //    }

    //    [&]<std::size_t... I>(std::index_sequence<I...>) {
    //        (([&] {
    //            constexpr raze::uint32 sh = I;
    //            auto slid = raze::datapar::slide_left(v, std::integral_constant<raze::uint32, sh>{});

    //            alignas(64) T expected[N];

    //            if constexpr (sh >= N) {
    //                for (size_t i = 0; i < N; ++i)
    //                    expected[i] = T(0);
    //            }
    //            else {
    //                for (size_t i = 0; i < N; ++i) {
    //                    if (i >= sh)
    //                        expected[i] = arr[i - sh];
    //                    else
    //                        expected[i] = T(0);
    //                }
    //            }

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(slid[i] == expected[i]);
    //            }()), ...);
    //    }(std::make_index_sequence<N + 1>{});

    //    for (raze::uint32 sh = 0; sh <= N; ++sh) {
    //        auto slid = raze::datapar::slide_right(v, sh);

    //        alignas(64) T expected[N];

    //        if (sh >= N) {
    //            for (size_t i = 0; i < N; ++i)
    //                expected[i] = T(0);
    //        }
    //        else {
    //            for (size_t i = 0; i < N; ++i) {
    //                if (i + sh < N)
    //                    expected[i] = arr[i + sh];
    //                else
    //                    expected[i] = T(0);
    //            }
    //        }

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(slid[i] == expected[i]);
    //    }

    //    [&]<std::size_t... I>(std::index_sequence<I...>) {
    //        (([&] {
    //            constexpr raze::uint32 sh = I;
    //            auto slid = raze::datapar::slide_right(v, std::integral_constant<raze::uint32, sh>{});

    //            alignas(64) T expected[N];

    //            if constexpr (sh >= N) {
    //                for (size_t i = 0; i < N; ++i)
    //                    expected[i] = T(0);
    //            }
    //            else {
    //                for (size_t i = 0; i < N; ++i) {
    //                    if (i + sh < N)
    //                        expected[i] = arr[i + sh];
    //                    else
    //                        expected[i] = T(0);
    //                }
    //            }

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(slid[i] == expected[i]);
    //            }()), ...);
    //    }(std::make_index_sequence<N + 1>{});
    //}

    //{
    //    alignas(64) T arr[N];
    //    for (size_t i = 0; i < N; ++i)
    //        arr[i] = static_cast<T>(i + 1);

    //    Simd v = raze::datapar::load<Simd>(arr);

    //    for (raze::uint32 sh = 0; sh < N; ++sh) {
    //        auto rotated = raze::datapar::rotate_left(v, sh);

    //        alignas(64) T expected[N];
    //        for (size_t i = 0; i < N; ++i)
    //            expected[i] = arr[(i + sh) % N];

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(rotated[i] == expected[i]);
    //    }

    //    [&]<std::size_t... I>(std::index_sequence<I...>) {
    //        (([&] {
    //            constexpr raze::uint32 sh = I;
    //            auto rotated = raze::datapar::rotate_left(v, std::integral_constant<raze::uint32, sh>{});

    //            alignas(64) T expected[N];
    //            for (size_t i = 0; i < N; ++i)
    //                expected[i] = arr[(i + sh) % N];

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(rotated[i] == expected[i]);
    //            }()), ...);
    //    }(std::make_index_sequence<N>{});

    //    for (raze::uint32 sh = 0; sh < N; ++sh) {
    //        auto rotated = raze::datapar::rotate_right(v, sh);

    //        alignas(64) T expected[N];
    //        for (size_t i = 0; i < N; ++i)
    //            expected[i] = arr[(i + N - sh) % N];

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(rotated[i] == expected[i]);
    //    }

    //    [&]<std::size_t... I>(std::index_sequence<I...>) {
    //        (([&] {
    //            constexpr raze::uint32 sh = I;
    //            auto rotated = raze::datapar::rotate_right(v, std::integral_constant<raze::uint32, sh>{});

    //            alignas(64) T expected[N];
    //            for (size_t i = 0; i < N; ++i)
    //                expected[i] = arr[(i + N - sh) % N];

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(rotated[i] == expected[i]);
    //            }()), ...);
    //    }(std::make_index_sequence<N>{});
    //}

    //{
    //    alignas(64) T arrA[N], arrB[N];
    //    for (size_t i = 0; i < N; ++i) {
    //        arrA[i] = static_cast<T>(i + 1);
    //        arrB[i] = static_cast<T>((i + 1) * 3);
    //    }

    //    Simd a = raze::datapar::load<Simd>(arrA);
    //    Simd b = raze::datapar::load<Simd>(arrB);

    //    {
    //        Simd v = a;
    //        v += b;

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] + arrB[i]));
    //    }

    //    {
    //        Simd v = a;
    //        v -= b;

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] - arrB[i]));
    //    }

    //    {
    //        Simd v = a;
    //        v *= b;

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] * arrB[i]));
    //    }

    //    {
    //        for (size_t i = 0; i < N; ++i)
    //            arrB[i] = static_cast<T>(i + 1);

    //        b = raze::datapar::load<Simd>(arrB);

    //        Simd v = a;
    //        v /= b;

    //        for (size_t i = 0; i < N; ++i)
    //            raze_assert(v[i] == static_cast<T>(arrA[i] / arrB[i]));
    //    }

    //    if constexpr (std::is_integral_v<T>) {
    //        for (raze::uint32 sh = 0; sh <= sizeof(T) * 8; ++sh) {
    //            {
    //                Simd v = a;
    //                v <<= sh;

    //                for (size_t i = 0; i < N; ++i) {
    //                    T expected = 0;
    //                    if (sh < sizeof(T) * 8)
    //                        expected = static_cast<T>(arrA[i] << sh);

    //                    raze_assert(v[i] == expected);
    //                }
    //            }

    //            {
    //                Simd v = a;
    //                v >>= sh;

    //                for (size_t i = 0; i < N; ++i) {
    //                    T expected = 0;

    //                    if (sh < sizeof(T) * 8)
    //                        expected = static_cast<T>(arrA[i] >> sh);
    //                    else if constexpr (std::is_signed_v<T>)
    //                        expected = static_cast<T>(arrA[i] >> (sizeof(T) * 8 - 1));

    //                    raze_assert(v[i] == expected);
    //                }
    //            }
    //        }
    //    }

    //    if constexpr (std::is_integral_v<T>) {
    //        {
    //            Simd v = a;
    //            v &= b;

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] & arrB[i]));
    //        }

    //        {
    //            Simd v = a;
    //            v |= b;

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] | arrB[i]));
    //        }

    //        {
    //            Simd v = a;
    //            v ^= b;

    //            for (size_t i = 0; i < N; ++i)
    //                raze_assert(v[i] == (arrA[i] ^ arrB[i]));
    //        }
    //    }
    //}

    using _IndexType = typename raze::IntegerForSizeof<T>::Unsigned;

    {
        alignas(64) T arr[N];
        for (size_t i = 0; i < N; ++i)
            arr[i] = static_cast<T>(i + 1);

        Simd v = raze::datapar::load<Simd>(arr);

        [&] <std::size_t... I>(std::index_sequence<I...>) {
            constexpr size_t M = sizeof...(I);
            auto r = raze::datapar::shuffle(v, std::integer_sequence<raze::uint64, (N - 1 - I)...>{});

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[N - 1 - i]);
        }(std::make_index_sequence<N>{});

        {
            alignas(64) _IndexType idxArr[N];
            for (size_t i = 0; i < N; ++i)
                idxArr[i] = static_cast<_IndexType>(i);

            auto idx = raze::datapar::load<raze::datapar::simd<_IndexType, typename Simd::abi_type>>(idxArr);
            auto r = raze::datapar::shuffle(v, idx);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[i]);
        }

        std::mt19937_64 rng(0xBADC0FFEEULL);
        std::uniform_int_distribution<uint32_t> dist(0, N - 1);

        for (int iter = 0; iter < 1000000; ++iter) {
            uint32_t repeated = dist(rng);

            alignas(64) _IndexType idxArr[N];
            for (size_t i = 0; i < N; ++i)
                idxArr[i] = repeated;

            auto idx = raze::datapar::load<raze::datapar::simd<_IndexType, typename Simd::abi_type>>(idxArr);
            auto r = raze::datapar::shuffle(v, idx);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[repeated]);
        }

        for (int iter = 0; iter < 1000000; ++iter) {
            uint32_t repeated = dist(rng);

            alignas(64) _IndexType idxArr[N];
            for (size_t i = 0; i < N; ++i)
                idxArr[i] = repeated;

            auto idx = raze::datapar::load<raze::datapar::simd<_IndexType, typename Simd::abi_type>>(idxArr);
            auto r = raze::datapar::shuffle(v, idx);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[repeated]);
        }

        for (int iter = 0; iter < 1000000; ++iter) {
            uint32_t a = dist(rng);
            uint32_t b = dist(rng);

            alignas(64) _IndexType idxArr[N];
            for (size_t i = 0; i < N; ++i)
                idxArr[i] = (a * i + b) % N;

            auto idx = raze::datapar::load<raze::datapar::simd<_IndexType, typename Simd::abi_type>>(idxArr);
            auto r = raze::datapar::shuffle(v, idx);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == arr[idxArr[i]]);
        }

        test_shuffle_with_compile_time_all_patterns<Simd>(std::make_index_sequence<128>{});
    }
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

    //test_methods<raze::arch::ISA::AVX2, 128>();
    //test_methods<raze::arch::ISA::AVX2, 256>();
    //
    //test_methods<raze::arch::ISA::AVX512F, 512>();
    //test_methods<raze::arch::ISA::AVX512BW, 512>();
    //test_methods<raze::arch::ISA::AVX512DQ, 512>();
    //test_methods<raze::arch::ISA::AVX512BWDQ, 512>();
    //test_methods<raze::arch::ISA::AVX512VBMI, 512>();
    //test_methods<raze::arch::ISA::AVX512VBMI2, 512>();
    //test_methods<raze::arch::ISA::AVX512VBMIDQ, 512>();
    //test_methods<raze::arch::ISA::AVX512VBMI2DQ, 512>();

    //test_methods<raze::arch::ISA::AVX512VLF, 128>();
    //test_methods<raze::arch::ISA::AVX512VLBW, 128>();
    //test_methods<raze::arch::ISA::AVX512VLBWDQ, 128>();
    //test_methods<raze::arch::ISA::AVX512VLDQ, 128>();

    //test_methods<raze::arch::ISA::AVX512VLF, 256>();
    //test_methods<raze::arch::ISA::AVX512VLBW, 256>();
    //test_methods<raze::arch::ISA::AVX512VLBWDQ, 256>();
    //test_methods<raze::arch::ISA::AVX512VLDQ, 256>();

    //test_methods<raze::arch::ISA::AVX512VBMIVL, 128>();
    //test_methods<raze::arch::ISA::AVX512VBMI2VL, 128>();
    //test_methods<raze::arch::ISA::AVX512VBMIVLDQ, 128>();
    //test_methods<raze::arch::ISA::AVX512VBMI2VLDQ, 128>();

    //test_methods<raze::arch::ISA::AVX512VBMIVL, 256>();
    //test_methods<raze::arch::ISA::AVX512VBMI2VL, 256>();
    //test_methods<raze::arch::ISA::AVX512VBMIVLDQ, 256>();
    //test_methods<raze::arch::ISA::AVX512VBMI2VLDQ, 256>();

    return 0;
}
