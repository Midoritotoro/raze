#include <raze/datapar/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>
#include <raze/algorithm/fill/Fill.h>
#include <string>
#include <vector>
#include <numeric>
#include <limits>
#include <algorithm>
#include <iostream>

template <typename T, raze::arch::ISA Arch, raze::uint32 _Width_>
void testSimdMaskMethods() {
    using Simd = raze::datapar::simd<Arch, T, _Width_>;
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
        // raze_assert(raze::datapar::none_of(all_false));
    }

    {
        //std::bitset<N> bits;
        //for (size_t i = 0; i < N; ++i)
        //    bits[i] = (i % 2 == 0);

        //Mask m(bits);
        //for (size_t i = 0; i < N; ++i)
        //    raze_assert(m[i] == (i % 2 == 0));
    }

    {
        /*bool src[N];
        bool dst[N];

        for (size_t i = 0; i < N; ++i)
            src[i] = (i % 3 == 0);*/

        //Mask m(src);
        //m.copy_to(dst);

        //for (size_t i = 0; i < N; ++i)
        //    raze_assert(dst[i] == src[i]);
    }

    {
        //Mask m(false);

        //m[N - 1] = true;
        //m[0] = true;

        //raze_assert(m[N - 1] == true);
        //raze_assert(m[0] == true);

        //for (size_t i = 0; i < N; ++i)
        //    if (i != 0 && i != N - 1)
        //        raze_assert(m[i] == false);
    }

    //{
    //    std::bitset<64> a_bits;
    //    std::bitset<64> b_bits;
    //    for (size_t i = 0; i < N; ++i) {
    //        a_bits[i] = (i % 2 == 0);
    //        b_bits[i] = (i % 3 == 0);
    //    }
    //    Mask a(a_bits);
    //    Mask b(b_bits);
    //    auto c_and = a & b;
    //    auto c_or = a | b;
    //    auto c_xor = a ^ b;
    //    auto c_not = !a;
    //    for (size_t i = 0; i < N; ++i) {
    //        bool av = a_bits[i];
    //        bool bv = b_bits[i];
    //        raze_assert(c_and[i] == (av && bv));
    //        raze_assert(c_or[i] == (av || bv));
    //        raze_assert(c_xor[i] == (av ^ bv));
    //        raze_assert(c_not[i] == (!av));
    //    }
    //}

    //{
    //    Mask all_true(true);
    //    Mask all_false(false);
    //    raze_assert(all_true.__all_of() == true);
    //    raze_assert(all_true.__none_of() == false);
    //    raze_assert(all_true.__some_of() == false);
    //    raze_assert(all_false.__none_of() == true);
    //    raze_assert(all_false.__any_of() == false);
    //    raze_assert(all_false.__some_of() == false);
    //    Mask mixed(false);
    //    if (N > 0) mixed[0] = true;
    //    raze_assert(mixed.__any_of() == true);
    //    raze_assert(mixed.__some_of() == true);
    //    raze_assert(mixed.__all_of() == false);
    //}

    //{
    //    Mask m(false);
    //    for (size_t i = 0; i < N; ++i)
    //        if (i % 2 == 0) m[i] = true;
    //    size_t expectedCount = (N + 1) / 2;
    //    raze_assert(m.__count_set() == static_cast<int32_t>(expectedCount));
    //    auto first_set = raze::datapar::find_first_set(m);
    //    auto last_set = raze::datapar::find_last_set(m);
    //    auto first_nset = raze::datapar::find_first_not_set(m);
    //    auto last_nset = raze::datapar::find_last_not_set(m);
    //    if (N > 0) raze_assert(first_set == 0);
    //    if (N > 0) {
    //        size_t expected_last = (N % 2 == 0 ? N - 2 : N - 1);
    //        raze_assert(last_set == expected_last);
    //    }
    //    if (N > 1) raze_assert(first_nset == 1);
    //    if (N > 0) raze_assert(last_nset == 0);
    //}

    {
        using M1 = Mask;
        using M2 = typename raze::datapar::simd<Arch, T, _Width_>::mask_type;
        using M3 = typename raze::datapar::simd<Arch, float, _Width_>::mask_type;
        using M4 = typename raze::datapar::simd<raze::arch::ISA::SSE2, T, 128>::mask_type;
        static_assert(raze::datapar::__compatible_mask<M1, M2>);
       // static_assert((raze::datapar::__compatible_mask<M1, M3>);
       // static_assert(!raze::datapar::__compatible_mask<M1, M4>);
    }
}

template <raze::arch::ISA _ISA_, raze::uint32 _Width_>
void testSimdMaskMethods() {
    /*testSimdMaskMethods<raze::int8, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint8, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int16, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint16, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int32, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint32, _ISA_, _Width_>();

    testSimdMaskMethods<raze::int64, _ISA_, _Width_>();
    testSimdMaskMethods<raze::uint64, _ISA_, _Width_>();*/

    testSimdMaskMethods<float, _ISA_, _Width_>();
    // testSimdMaskMethods<double, _ISA_, _Width_>();
}


int main() {
    //testSimdMaskMethods<raze::arch::ISA::SSE2, 128>();
    //testSimdMaskMethods<raze::arch::ISA::SSE3, 128>();
    //testSimdMaskMethods<raze::arch::ISA::SSSE3, 128>();
    //testSimdMaskMethods<raze::arch::ISA::SSE41, 128>();
    //testSimdMaskMethods<raze::arch::ISA::SSE42, 128>();

    //testSimdMaskMethods<raze::arch::ISA::AVX2, 128>();
    //testSimdMaskMethods<raze::arch::ISA::AVX2, 256>();

    testSimdMaskMethods<raze::arch::ISA::AVX512F, 512>();
    /*testSimdMaskMethods<raze::arch::ISA::AVX512BW, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512DQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512BWDQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMIDQ, 512>();
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2DQ, 512>();*/
    /*
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
    testSimdMaskMethods<raze::arch::ISA::AVX512VBMI2VLDQ, 256>();*/

    return 0;
}
