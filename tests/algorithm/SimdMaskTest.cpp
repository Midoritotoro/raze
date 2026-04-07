#include <raze/datapar/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>
#include <raze/algorithm/fill/Fill.h>
#include <string>
#include <random>
#include <vector>
#include <numeric>
#include <limits>
#include <algorithm>
#include <iostream>

template <typename T, raze::arch::ISA Arch, raze::uint32 _Width_>
void testSimdMaskMethods() {
    using Simd = raze::datapar::simd<T, raze::datapar::x86_abi<Arch, _Width_>>;
    using Mask = typename Simd::mask_type;
    constexpr size_t N = Simd::size();

    if (!raze::arch::ProcessorFeatures::isSupported<Simd::__isa>()) {
        std::cout << "Mask not supported for ISA=" << int(Simd::__isa) << '\n';
        return;
    }

    {
        Mask all_true(true);
        Mask all_false(false);
        
        raze_assert(raze::datapar::all_of(all_true));
        raze_assert(raze::datapar::none_of(all_false));
    }

    {
        std::bitset<N> bits;
        for (size_t i = 0; i < N; ++i)
            bits[i] = (i % 2 == 0);

        Mask m(bits);
        for (size_t i = 0; i < N; ++i)
            raze_assert(m[i] == (i % 2 == 0));
    }

    {
        bool src[N];
        bool dst[N];

        for (size_t i = 0; i < N; ++i)
            src[i] = (i % 3 == 0);

        Mask m(src);
        m.copy_to(dst);

        for (size_t i = 0; i < N; ++i)
            raze_assert(dst[i] == src[i]);
    }

    {
        Mask m(false);

        m[N - 1] = true;
        m[0] = true;

        raze_assert(m[N - 1] == true);
        raze_assert(m[0] == true);

        for (size_t i = 0; i < N; ++i)
            if (i != 0 && i != N - 1)
                raze_assert(m[i] == false);
    }

    {
        std::bitset<64> a_bits;
        std::bitset<64> b_bits;
        for (size_t i = 0; i < N; ++i) {
            a_bits[i] = (i % 2 == 0);
            b_bits[i] = (i % 3 == 0);
        }
        Mask a(a_bits);
        Mask b(b_bits);
        auto c_and = a & b;
        auto c_or = a | b;
        auto c_xor = a ^ b;
        auto c_not = !a;
        for (size_t i = 0; i < N; ++i) {
            bool av = a_bits[i];
            bool bv = b_bits[i];
            raze_assert(c_and[i] == (av && bv));
            raze_assert(c_or[i] == (av || bv));
            raze_assert(c_xor[i] == (av ^ bv));
            raze_assert(c_not[i] == (!av));
        }
    }

    {
        Mask all_true(true);
        Mask all_false(false);
        raze_assert(raze::datapar::all_of(all_true));
        raze_assert(raze::datapar::none_of(all_true) == false);
        raze_assert(raze::datapar::some_of(all_true) == false);
        raze_assert(raze::datapar::none_of(all_false));
        raze_assert(raze::datapar::any_of(all_false) == false);
        raze_assert(raze::datapar::some_of(all_false));
        Mask mixed(false);
        if (N > 0) mixed[0] = true;
        raze_assert(raze::datapar::any_of(mixed));
        raze_assert(raze::datapar::some_of(mixed));
        raze_assert(raze::datapar::all_of(mixed) == false);
    }

    auto makeMask = [N](std::initializer_list<size_t> bits) {
        Mask m{};

        for (size_t i = 0; i < N; ++i)
            m[i] = false;

        for (size_t b : bits)
            if (b < N)
                m[b] = true;

        return m;
        };

    {
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

    {
        {
            Mask m{};
            for (size_t i = 0; i < N; ++i) m[i] = false;

            raze_assert(raze::datapar::find_first_not_set(m) == 0);
            raze_assert(raze::datapar::find_last_not_set(m) == 0);
        }

        {
            Mask m{};
            for (size_t i = 0; i < N; ++i) m[i] = true;

            raze_assert(raze::datapar::find_first_not_set(m) == N);
            raze_assert(raze::datapar::find_last_not_set(m) == N);
        }

        for (size_t i = 0; i < N; ++i) {
            Mask m{};
            for (size_t j = 0; j < N; ++j) m[j] = true;
            m[i] = false;

            raze_assert(raze::datapar::find_first_not_set(m) == i);
            raze_assert(raze::datapar::find_last_not_set(m) == (N - i - 1));
        }

        {
            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            size_t expectedFirst = (N > 1 ? 1 : N);

            raze_assert(raze::datapar::find_first_not_set(m) == expectedFirst);
            raze_assert(raze::datapar::find_last_not_set(m) == 0);
        }

        {
            std::initializer_list<size_t> ilist = { 1,3,4,6,7,8,9,11,14 };
            Mask m = makeMask(ilist);

            size_t expectedFirst = 0;
            while (expectedFirst < N && m[expectedFirst]) ++expectedFirst;

            size_t expectedLast = N;
            for (size_t i = 0; i < N; ++i)
                if (!m[i]) expectedLast = N - i - 1;

            raze_assert(raze::datapar::find_first_not_set(m) == expectedFirst);
            raze_assert(raze::datapar::find_last_not_set(m) == expectedLast);
        }
    }

    {
        using M1 = Mask;
        using M2 = typename raze::datapar::simd<T, raze::datapar::x86_abi<Arch, _Width_>>::mask_type;
        using M3 = typename raze::datapar::simd<typename raze::IntegerForSizeof<T>::Unsigned, raze::datapar::x86_abi<Arch, _Width_>>::mask_type;
        static_assert(raze::datapar::__compatible_mask<M1, M2>);
        static_assert(raze::datapar::__compatible_mask<M1, M3>);
        static_assert(raze::datapar::__compatible_mask<M2, M3>);
    }

    {
        {
            Mask m(false);
            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(!raze::datapar::is_contiguous(m, n, k));
        }

        {
            Mask m(true);
            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(raze::datapar::is_contiguous(m, n, k));
        }

        for (size_t pos = 0; pos < N; ++pos) {
            Mask m(false);
            m[pos] = true;

            for (size_t n = 0; n < N; ++n) {
                for (size_t k = n; k < N; ++k) {
                    bool expected = (n == pos && k == pos);
                    raze_assert(raze::datapar::is_contiguous(m, n, k) == expected);
                }
            }
        }

        {
            Mask m(false);
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            for (size_t n = 0; n < N; ++n) {
                for (size_t k = n; k < N; ++k) {
                    bool expected = true;
                    for (size_t i = n; i <= k; ++i)
                        if (!m[i]) expected = false;

                    raze_assert(raze::datapar::is_contiguous(m, n, k) == expected);
                }
            }
        }

        {
            std::mt19937 rng(123456);
            for (int iter = 0; iter < 1000; ++iter) {
                Mask m(false);
                for (size_t i = 0; i < N; ++i)
                    m[i] = (rng() & 1);

                for (size_t n = 0; n < N; ++n) {
                    for (size_t k = n; k < N; ++k) {
                        bool expected = true;
                        for (size_t i = n; i <= k; ++i)
                            if (!m[i]) expected = false;

                   
                        raze_assert(raze::datapar::is_contiguous(m, n, k) == expected);
                    }
                }
            }
        }
    }

    {
        for (size_t sh = 0; sh <= N; ++sh) {
            Mask m{};

            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            auto r = m << sh;

            for (size_t i = 0; i < N; ++i) {
                T expected = (i + sh < N) ? m[i + sh] : T(0);
                raze_assert(r[i] == expected);
            }
        }

        for (size_t sh = 0; sh <= N; ++sh) {
            Mask m{};

            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            auto r = m >> sh;

            for (size_t i = 0; i < N; ++i) {
                T expected = (i >= sh) ? m[i - sh] : T(0);
                raze_assert(r[i] == expected);
            }
        }

        for (raze::uint32 sh = 0; sh <= N; ++sh) {
            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 1);

            Mask r = m;
            r <<= sh;

            for (size_t i = 0; i < N; ++i) {
                T expected = (i + sh < N) ? m[i + sh] : T(0);
                raze_assert(r[i] == expected);
            }
        }

        for (raze::uint32 sh = 0; sh <= N; ++sh) {
            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 4 == 0);

            Mask r = m;
            r >>= sh;

            for (size_t i = 0; i < N; ++i) {
                T expected = (i >= sh) ? m[i - sh] : T(0);
                raze_assert(r[i] == expected);
            }
        }
    }

    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (([&] {
            constexpr raze::uint32 sh = I;

            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 5 == 0);

            Mask r = (m >> std::integral_constant<raze::uint32, sh>{});

            for (size_t i = 0; i < N; ++i) {
                T expected = (i >= sh) ? m[i - sh] : T(0);
                raze_assert(r[i] == expected);
            }
            }()), ...);
    }(std::make_index_sequence<N + 1>{});

    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (([&] {
            constexpr raze::uint32 sh = I;

            Mask m{};
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 7 == 0);

            Mask r = (m << std::integral_constant<raze::uint32, sh>{});

            for (size_t i = 0; i < N; ++i) {
                T expected = (i + sh < N) ? m[i + sh] : T(0);
                raze_assert(r[i] == expected);
            }
            }()), ...);
    }(std::make_index_sequence<N + 1>{});
}

template <raze::arch::ISA _ISA_, raze::uint32 _Width_>
void testSimdMaskMethods() {
    testSimdMaskMethods<raze::int8, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint8, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int16, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint16, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int32, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint32, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int64, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint64, _ISA_, _Width_>();

    testSimdMaskMethods<float, _ISA_, _Width_>();
    testSimdMaskMethods<double, _ISA_, _Width_>();
}


int main() {
    testSimdMaskMethods<raze::arch::ISA::SSE2, 128>();
    testSimdMaskMethods<raze::arch::ISA::SSE3, 128>();
    testSimdMaskMethods<raze::arch::ISA::SSSE3, 128>();
    testSimdMaskMethods<raze::arch::ISA::SSE41, 128>();
    testSimdMaskMethods<raze::arch::ISA::SSE42, 128>();

    testSimdMaskMethods<raze::arch::ISA::AVX2, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX2, 256>();

    testSimdMaskMethods<raze::arch::ISA::AVX512F, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512BW, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512DQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512BWDQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIDQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2DQ, 512>();
    
    testSimdMaskMethods<raze::arch::ISA::AVX512VLF, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLBW, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLBWDQ, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLDQ, 128>();

    testSimdMaskMethods<raze::arch::ISA::AVX512VLF, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLBW, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLBWDQ, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VLDQ, 256>();

    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIVL, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2VL, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIVLDQ, 128>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2VLDQ, 128>();

    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIVL, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2VL, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIVLDQ, 256>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2VLDQ, 256>();

    return 0;
}
