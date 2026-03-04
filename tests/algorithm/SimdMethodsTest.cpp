#include <raze/datapar/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>
#include <raze/algorithm/fill/Fill.h>
#include <string>


template <typename _Simd_>
void mask_compress_any(
    const typename _Simd_::value_type* a,
    const typename _Simd_::value_type* src,
    typename _Simd_::value_type* dst,
    typename _Simd_::mask_type mask)
{
    constexpr auto N = _Simd_::template size();

    int m = 0;

    for (int j = 0; j < N; ++j)
        if (!mask[j])
            dst[m++] = a[j];

    for (int i = m; i < N; ++i)
        dst[i] = src[i];
}

template <typename T, raze::arch::ISA Arch, raze::uint32 _Width_>
void testMethods() {
    using Simd = raze::datapar::simd<Arch, T, _Width_>;
    using Mask = typename Simd::mask_type;
    constexpr size_t N = Simd::size();

    if (raze::arch::ProcessorFeatures::isSupported<Simd::__isa>() == false) {
        std::cout << "Not supported" << '\n';
        return;
    }

     {
        Simd v1;
        Simd v2(5);

        for (int i = 0; i < v2.size(); ++i) raze_assert(v2[i] == 5);

        alignas(16) T arr[N];
        std::iota(arr, arr + N, 1);

        Simd v3 = raze::datapar::load<Simd>(arr);
        for (int i = 0; i < v2.size(); ++i)
            raze_assert(v3[i] == arr[i]);


        Simd v4(v3);
        for (int i = 0; i < v2.size(); ++i) raze_assert(v4[i] == arr[i]);

        Simd v5(v3);
        for (int i = 0; i < v2.size(); ++i) raze_assert(v5[i] == arr[i]);
    }

    {
        Simd v;
        v.fill(42);
        for (int i = 0; i < v.size(); ++i) raze_assert(v[i] == 42);

        v[0] = 99;
        raze_assert(v[0] == 99);
    }

    {
        using raze::datapar::simd_cast;
        Simd v(11);
        auto vOther = simd_cast<raze::datapar::simd128_sse2<float>>(v);
        auto vOther2 = simd_cast<raze::arch::ISA::SSE2, float>(v);
        auto vOther3 = simd_cast<raze::arch::ISA::SSE2>(v);
        auto vOther4 = simd_cast<__m128i>(v);
        auto vOther5 = simd_cast<int>(v);

        static_assert(std::is_same_v<decltype(vOther), decltype(vOther2)>);
        static_assert(std::is_same_v<decltype(vOther2), raze::datapar::simd128_sse2<float>>);
        static_assert(std::is_same_v<decltype(vOther3), raze::datapar::simd<raze::arch::ISA::SSE2, typename Simd::value_type, 128>>);
        static_assert(std::is_same_v<decltype(vOther4), __m128i>);
        static_assert(std::is_same_v<decltype(vOther5), raze::datapar::simd<Simd::__isa, int, Simd::__width>>);
    }

    {
        alignas(64) T src[N];
        alignas(64) T dst[N];

        for (size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i + 1);
        for (size_t i = 0; i < N; ++i) dst[i] = static_cast<T>(100 + i);

        Mask mask = 0;
        //for (size_t i = 0; i < N; ++i)
        //    if (i % 2 == 0)
        //        mask |= (Mask(1) << i);

        Simd loaded_unaligned = raze::datapar::maskz_load<Simd>(src, mask);
        for (size_t i = 0; i < N; ++i) {
            if (mask[i])
                raze_assert(loaded_unaligned[i] == src[i]);
            else
                raze_assert(loaded_unaligned[i] == T(0));
        }

        Simd loaded_aligned = raze::datapar::maskz_load<Simd>(src, mask, raze::datapar::aligned_policy{});
        for (size_t i = 0; i < N; ++i) {
            if (mask[i])
                raze_assert(loaded_aligned[i] == src[i]);
            else
                raze_assert(loaded_aligned[i] == T(0));
        }

        Simd v(77);
        raze::datapar::mask_store(dst, v, mask);
        for (size_t i = 0; i < N; ++i) {
            if (mask[i])
                raze_assert(dst[i] == T(77));
            else
                raze_assert(dst[i] == T(100 + i));
        }

        for (size_t i = 0; i < N; ++i) dst[i] = static_cast<T>(200 + i);
        raze::datapar::mask_store(dst, v, mask, raze::datapar::aligned_policy{});
        for (size_t i = 0; i < N; ++i) {
            if (mask[i])
                raze_assert(dst[i] == T(77));
            else
                raze_assert(dst[i] == T(200 + i));
        }
    }
    
    {
        alignas(64) T src[N];
        for (size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i + 1);

        Simd v = raze::datapar::load<Simd>(src);


        Mask mask = 0;
        //for (size_t i = 0; i < N; i += 2)
        //    mask |= (Mask(1) << i);


        {
            alignas(64) T dst[N] = {};
            raze::datapar::compress_store(dst, v, mask, raze::datapar::aligned_policy{});

            alignas(64) T expected[N];
            mask_compress_any<Simd>(src, src, expected, mask);

            raze_assert(std::equal(expected, expected + N, dst));
        }

        {
            alignas(64) T dst[N] = {};
            raze::datapar::compress_store(dst, v, mask);
                
            alignas(64) T expected[N];
            mask_compress_any<Simd>(src, src, expected, mask);

            raze_assert(std::equal(expected, expected + N, dst));
        }

        {
            alignas(64) T dst[N] = {};
            raze::datapar::compress_store(dst, v, mask, raze::datapar::aligned_policy{});

            alignas(64) T expected[N];
            mask_compress_any<Simd>(src, src, expected, mask);

            raze_assert(std::equal(expected, expected + N, dst));
        }
    }

    {
        std::vector<T> va(N), vb(N), vc(N);
        for (size_t i = 0; i < N; ++i) {
            va[i] = static_cast<T>(i + 1);
            vb[i] = static_cast<T>(i + 1);
            vc[i] = static_cast<T>(i + 2);
        }

        Simd a = raze::datapar::load<Simd>(va.data());
        Simd b = raze::datapar::load<Simd>(vb.data());
        Simd c = raze::datapar::load<Simd>(vc.data());

        raze::datapar::simd<raze::arch::ISA::SSE2, int, 128> v1;
        raze::datapar::simd<raze::arch::ISA::SSE2, int, 128> v2;
        auto m = v1 == v2;
        raze_assert(raze::datapar::all_of(m));
        raze_assert(raze::datapar::any_of(m));

        auto mEq = (a == b);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(mEq[i] == true);
        }

        auto mNeq = (a != c);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(mNeq[i] == true);
        }

        auto mGt = (c > a);
        auto mLt = (a < c);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(mGt[i] == true);
            raze_assert(mLt[i] == true);
        }

        auto mGe = (a >= b);
        auto mLe = (a <= b);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(mGe[i] == true);
            raze_assert(mLe[i] == true);
        }
    }

    {
        alignas(64) T arr0[N], arrMax[N];
        for (size_t i = 0; i < N; ++i) {
            arr0[i] = 0;
            arrMax[i] = std::numeric_limits<T>::max();
        }

        Simd v0 = raze::datapar::load<Simd>(arr0);
        Simd vmax = raze::datapar::load<Simd>(arrMax);

        raze_assert(raze::datapar::any_of(v0 != vmax));

        auto mEq = (v0 == v0);
        auto mNeq = (v0 != vmax);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(mEq[i] == true);
            raze_assert(mNeq[i] == true);
        }

        auto mLt = (v0 < vmax);
        auto mGt = (vmax > v0);
        auto mLe = (v0 <= v0);
        auto mGe = (vmax >= vmax);

        for (size_t i = 0; i < N; ++i) {
            raze_assert(mLt[i] == true);
            raze_assert(mGt[i] == true);
            raze_assert(mLe[i] == true);
            raze_assert(mGe[i] == true);
        }
    }

    {
        for (long long step = 1; step < (1LL << (std::numeric_limits<T>::digits - 2)); step <<= 1) {
            alignas(64) T arrA[N], arrB[N];
            for (size_t i = 0; i < N; ++i) {
                arrA[i] = step;
                arrB[i] = step + 1;
            }
            Simd vA = raze::datapar::load<Simd>(arrA);
            Simd vB = raze::datapar::load<Simd>(arrB);

            auto mEq = (vA == vA);
            auto mNeq = (vA != vB);
            auto mLt = (vA < vB);
            auto mGt = (vB > vA);
            auto mLe = (vA <= vA);
            auto mGe = (vB >= vB);

            for (size_t i = 0; i < N; ++i) {
                raze_assert(mEq[i] == true);
                raze_assert(mNeq[i] == true);
                raze_assert(mLt[i] == true);
                raze_assert(mGt[i] == true);
                raze_assert(mLe[i] == true);
                raze_assert(mGe[i] == true);
            }
        }
    }
  
    {
        raze::datapar::__reduce_type<T> reduced = 0;

        alignas(64) T array[N] = {};
        for (auto i = 0; i < N; ++i) {
            array[i] = (unsigned char)(i * 0x7fdbu);
        }

        for (auto i = 0; i < N; ++i) {
            reduced += (unsigned char)(array[i]);
        }

        auto simdReduced = raze::datapar::reduce(raze::datapar::load<Simd>(array));

        raze_assert(simdReduced == reduced);
    }


    {
        alignas(64) T arrA[N], arrB[N];

        for (size_t i = 0; i < N; ++i) {
            if (i % 4 == 0)
                arrA[i] = static_cast<T>(100);
            else if (i % 5 == 0)
                arrA[i] = static_cast<T>(-50);
            else
                arrA[i] = static_cast<T>(i - 2);

            if (i % 3 == 0)
                arrB[i] = static_cast<T>(200);
            else if (i % 7 == 0)
                arrB[i] = static_cast<T>(-100);
            else
                arrB[i] = static_cast<T>(N - i);
        }

        Simd a = raze::datapar::load<Simd>(arrA);
        Simd b = raze::datapar::load<Simd>(arrB);

        auto minVec = raze::datapar::vertical_min(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(minVec[i] == std::min(arrA[i], arrB[i]));

        auto maxVec = raze::datapar::vertical_max(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(maxVec[i] == std::max(arrA[i], arrB[i]));

        T minScalar = raze::datapar::horizontal_min(a);
        T expectedMin = *std::min_element(arrA, arrA + N);
        raze_assert(minScalar == expectedMin);

        T maxScalar = raze::datapar::horizontal_max(a);
        T expectedMax = *std::max_element(arrA, arrA + N);
        raze_assert(maxScalar == expectedMax);

        auto absVec = raze::datapar::abs(a);
        for (size_t i = 0; i < N; ++i) {
            raze_assert(absVec[i] == static_cast<T>(raze::math::abs(arrA[i])));
        }
    }

    {
        alignas(64) T arr[N];
        alignas(64) T reversed[N];

        std::iota(arr, arr + N, 0);

        auto v = raze::datapar::reverse(raze::datapar::load<Simd>(arr));
        raze::datapar::store(reversed, v);

        for (size_t i = 0; i < N; ++i)
            raze_assert(reversed[i] == arr[N - 1 - i]);
    }

    for (size_t i = 0; i < N; ++i) {
        Simd v1(0);
        Simd v2(0);

        v1[i] = 42;
        v2[i] = 42;

        auto mask = (v1 == v2);

        raze_assert(raze::datapar::any_of(mask));
        raze_assert(raze::datapar::all_of(mask));

        raze_assert(raze::datapar::count_set(mask) == N);
        raze_assert(raze::datapar::find_first_set(mask) == 0);
        raze_assert(raze::datapar::find_last_set(mask) == 0);
    }

    {
        auto makeMask = [N] (std::initializer_list<size_t> bits) {
            Mask m{};

            for (size_t i = 0; i < N; ++i)
                m[i] = false;

            for (size_t b : bits)
                if (b < N)
                    m[b] = true;

            return m;
        };

        {
            Mask m{};
            for (size_t i = 0; i < N; ++i) m[i] = false;

            raze_assert(raze::datapar::count_set(m) == 0);
            raze_assert(!raze::datapar::any_of(m));
            raze_assert(!raze::datapar::all_of(m));
            raze_assert(raze::datapar::find_first_set(m) == N);
            raze_assert(raze::datapar::find_last_set(m) == N);
        }


        {
            Mask m{};
            for (size_t i = 0; i < N; ++i) m[i] = true;

            raze_assert(raze::datapar::count_set(m) == N);
            raze_assert(raze::datapar::any_of(m));
            raze_assert(raze::datapar::all_of(m));
            raze_assert(raze::datapar::find_first_set(m) == 0);
            raze_assert(raze::datapar::find_last_set(m) == 0);
        }

        for (size_t i = 0; i < N; ++i) {
            Mask m{};
            for (size_t j = 0; j < N; ++j) m[j] = false;
            m[i] = true;

            raze_assert(raze::datapar::count_set(m) == 1);
            raze_assert(raze::datapar::any_of(m));
            raze_assert(!raze::datapar::all_of(m));
            raze_assert(raze::datapar::find_first_set(m) == i);
            auto f = raze::datapar::find_last_set(m);
            raze_assert(f == N - i - 1);
        }


        {
            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            size_t expectedCount = (N + 1) / 2;

            raze_assert(raze::datapar::count_set(m) == expectedCount);
            raze_assert(raze::datapar::any_of(m));
            raze_assert(!raze::datapar::all_of(m));
            raze_assert(raze::datapar::find_first_set(m) == 0);

            size_t last = (N % 2 == 0 ? 1 : N - 1);
            raze_assert(raze::datapar::find_last_set(m) == last);
        }


        {
            std::initializer_list<size_t> ilist = { 1, 3, 4, 6, 7, 8, 9, 11, 14 };
            Mask m = makeMask(ilist);

            size_t expectedCount = std::count_if(ilist.begin(), ilist.end(), [N](int val) { return val < N; });

            raze_assert(raze::datapar::count_set(m) == expectedCount);
            raze_assert(raze::datapar::find_first_set(m) == 1);
            raze_assert(raze::datapar::any_of(m));
            raze_assert(!raze::datapar::all_of(m));
        }
    }

}

template <raze::arch::ISA _ISA_, raze::uint32 _Width_>
void testMethods() {
    testMethods<raze::int8, _ISA_, _Width_>();
    testMethods<raze::uint8, _ISA_, _Width_>();

    testMethods<raze::int16, _ISA_, _Width_>();
    testMethods<raze::uint16, _ISA_, _Width_>();

    testMethods<raze::int32, _ISA_, _Width_>();
    testMethods<raze::uint32, _ISA_, _Width_>();

    testMethods<raze::int64, _ISA_, _Width_>();
    testMethods<raze::uint64, _ISA_, _Width_>();

    testMethods<float, _ISA_, _Width_>();
    testMethods<double, _ISA_, _Width_>();
}

int main() {
    testMethods<raze::arch::ISA::SSE2, 128>();
    testMethods<raze::arch::ISA::SSE3, 128>();
    testMethods<raze::arch::ISA::SSSE3, 128>();
    testMethods<raze::arch::ISA::SSE41, 128>();
    testMethods<raze::arch::ISA::SSE42, 128>();

    testMethods<raze::arch::ISA::AVX2, 128>();
    testMethods<raze::arch::ISA::AVX2, 256>();

    testMethods<raze::arch::ISA::AVX512F, 512>();
    testMethods<raze::arch::ISA::AVX512BW, 512>();
    testMethods<raze::arch::ISA::AVX512DQ, 512>();
    testMethods<raze::arch::ISA::AVX512BWDQ, 512>();
    testMethods<raze::arch::ISA::AVX512VBMI, 512>();
    testMethods<raze::arch::ISA::AVX512VBMI2, 512>();
    testMethods<raze::arch::ISA::AVX512VBMIDQ, 512>();
    testMethods<raze::arch::ISA::AVX512VBMI2DQ, 512>();

    testMethods<raze::arch::ISA::AVX512VLF, 128>();
    testMethods<raze::arch::ISA::AVX512VLBW, 128>();
    testMethods<raze::arch::ISA::AVX512VLBWDQ, 128>();
    testMethods<raze::arch::ISA::AVX512VLDQ, 128>();
    
    testMethods<raze::arch::ISA::AVX512VLF, 256>();
    testMethods<raze::arch::ISA::AVX512VLBW, 256>();
    testMethods<raze::arch::ISA::AVX512VLBWDQ, 256>();
    testMethods<raze::arch::ISA::AVX512VLDQ, 256>();

    testMethods<raze::arch::ISA::AVX512VBMIVL, 128>();
    testMethods<raze::arch::ISA::AVX512VBMI2VL, 128>();
    testMethods<raze::arch::ISA::AVX512VBMIVLDQ, 128>();
    testMethods<raze::arch::ISA::AVX512VBMI2VLDQ, 128>();

    testMethods<raze::arch::ISA::AVX512VBMIVL, 256>();
    testMethods<raze::arch::ISA::AVX512VBMI2VL, 256>();
    testMethods<raze::arch::ISA::AVX512VBMIVLDQ, 256>();
    testMethods<raze::arch::ISA::AVX512VBMI2VLDQ, 256>();

    return 0;
}
