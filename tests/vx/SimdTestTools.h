#pragma once 

#include <raze/vx/Simd.h>
#include <random>

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

template <
    class T, 
    size_t N,
    class Simd, 
    class Mask,
    class WhereExpr,
    class WhereZeroExpr,
    class SimdOp,
    class ScalarOp>
void test_where_ternary(
    const T(&arrA)[N], 
    const T(&arrB)[N], 
    const T(&arrC)[N],
    const Mask& m,
    const Simd& a, 
    const Simd& b, 
    const Simd& c,
    const Simd& src,
    const WhereExpr& w,
    const WhereZeroExpr& wz,
    SimdOp simd_op,
    ScalarOp scalar_op) noexcept
{
    {
        auto r1 = simd_op(w, b, c);
        auto r2 = simd_op(a, w, c);
        auto r3 = simd_op(a, b, w);

        for (size_t i = 0; i < N; ++i) {
            T expected1 = scalar_op(arrA[i], arrB[i], arrC[i], src[i], m[i], 0);
            T expected2 = scalar_op(arrA[i], arrA[i], arrC[i], src[i], m[i], 1);
            T expected3 = scalar_op(arrA[i], arrB[i], arrA[i], src[i], m[i], 2);

            raze_assert(r1[i] == expected1);
            raze_assert(r2[i] == expected2);
            raze_assert(r3[i] == expected3);
        }
    }

    {
        auto r1 = simd_op(wz, b, c);
        auto r2 = simd_op(a, wz, c);
        auto r3 = simd_op(a, b, wz);

        for (size_t i = 0; i < N; ++i) {
            T zero = T(0);

            T expected1 = m[i] ? scalar_op(arrA[i], arrB[i], arrC[i], zero, true, 0) : zero;
            T expected2 = m[i] ? scalar_op(arrA[i], arrA[i], arrC[i], zero, true, 1) : zero;
            T expected3 = m[i] ? scalar_op(arrA[i], arrB[i], arrA[i], zero, true, 2) : zero;

            raze_assert(r1[i] == expected1);
            raze_assert(r2[i] == expected2);
            raze_assert(r3[i] == expected3);
        }
    }
}


template <
    class T, 
    size_t N, 
    class Simd,
    class Mask,
    class WhereExpr, 
    class WhereZeroExpr, 
    class SimdOp, 
    class ScalarOp>
void test_where_binary(
    const T(&arrA)[N], 
    const T(&arrB)[N], 
    const T(&arrSrc)[N],
    const Mask& m, 
    const Simd& a,
    const Simd& b,
    const Simd& src,
    const WhereExpr& w,
    const WhereZeroExpr& wz, 
    SimdOp simd_op, 
    ScalarOp scalar_op) noexcept
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

template <
    class T, 
    size_t N, 
    class Simd,
    class Mask,
    class WhereExpr,
    class WhereZeroExpr, 
    class SimdOp,
    class ScalarOp>
void test_where_unary(
    const T(&arrA)[N],
    const T(&arrSrc)[N],
    const Mask& m,
    const Simd& a,
    const Simd& src,
    const WhereExpr& w,
    const WhereZeroExpr& wz,
    SimdOp simd_op, 
    ScalarOp scalar_op) noexcept
{
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
    }
}

template <
    template <class, raze::arch::ISA, raze::uint32> class _Function_,
    raze::arch::ISA  _ISA_,
    raze::uint32 _Width_>
void __test_all_helper() {
    if (!raze::arch::ProcessorFeatures::isSupported<_ISA_>())
        return;

    raze::type_traits::invoke(_Function_<raze::int8, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::uint8, _ISA_, _Width_>());

    raze::type_traits::invoke(_Function_<raze::int16, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::uint16, _ISA_, _Width_>());

    raze::type_traits::invoke(_Function_<raze::int32, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::uint32, _ISA_, _Width_>());

    raze::type_traits::invoke(_Function_<raze::int64, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::uint64, _ISA_, _Width_>());

    raze::type_traits::invoke(_Function_<float, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<double, _ISA_, _Width_>());
}

template <template <class, raze::arch::ISA, raze::uint32> class _Function_>
void test_all() { 
    __test_all_helper<_Function_, raze::arch::ISA::SSE2, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::SSE3, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::SSSE3, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::SSE41, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::SSE42, 128>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::FMA3, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX2, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX2FMA3, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX2, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::FMA3, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX2FMA3, 256>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512F, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512BW, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512DQ, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512BWDQ, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMIDQ, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2DQ, 512>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLF, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLBW, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLBWDQ, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLDQ, 128>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLF, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLBW, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLBWDQ, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLDQ, 256>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMIVL, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2VL, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMIVLDQ, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2VLDQ, 128>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMIVL, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2VL, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMIVLDQ, 256>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VBMI2VLDQ, 256>();
}
