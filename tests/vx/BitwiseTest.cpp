#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct bitwise_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    template<class SimdOp, class ScalarOp>
    static void run(const Simd& a, const Simd& b,
                    const _Type_(&A)[N], const _Type_(&B)[N],
                    SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(r[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])), std::bit_cast<U>(_Type_(B[i])))));
    }

    template<class SimdOp, class ScalarOp>
    static void run_unary(const Simd& a,
                          const _Type_(&A)[N],
                          SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(r[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])))));
    }

    template<class SimdOp, class ScalarOp>
    static void run_assign(const Simd& a0, const Simd& b,
                           const _Type_(&A)[N], const _Type_(&B)[N],
                           SimdOp sop, ScalarOp sc)
    {
        Simd a = a0;
        sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(a[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])), std::bit_cast<U>(_Type_(B[i])))));
    }

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N];

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_((i * 0x5A5A) ^ (i + 3));
            arrB[i] = _Type_((N - i) * 0x33 ^ (i * 7));
        }

        arrA[0]     = -1;
        arrA[N - 1] = 255;

        Simd a; a.copy_from(arrA);
        Simd b; b.copy_from(arrB);

        run(a, b, arrA, arrB, [](auto x, auto y){ return x & y; }, [](auto x, auto y){ return x & y; });
        run(a, b, arrA, arrB, [](auto x, auto y){ return x | y; }, [](auto x, auto y){ return x | y; });
        run(a, b, arrA, arrB, [](auto x, auto y){ return x ^ y; }, [](auto x, auto y){ return x ^ y; });

        run_unary(a, arrA, [](auto x){ return ~x; }, [](auto x){ return ~x; });

        run(a, b, arrA, arrB,
            [](auto x, auto y){ return (x & y) ^ (~x | y); },
            [](auto x, auto y){ return (x & y) ^ (~x | y); });

        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x &= y; }, [](auto x, auto y){ return x & y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x |= y; }, [](auto x, auto y){ return x | y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x ^= y; }, [](auto x, auto y){ return x ^ y; });

        if constexpr (std::is_integral_v<_Type_>) {
            for (raze::uint32 sh = 0; sh <= sizeof(_Type_) * 8; ++sh) {
                {
                    Simd v = a;
                    v <<= sh;

                    for (size_t i = 0; i < N; ++i) {
                        _Type_ scalar = 0;
                        if (sh < sizeof(_Type_) * 8)
                            scalar = std::bit_cast<U>(_Type_(arrA[i])) << sh;

                        raze_assert(v[i] == scalar);
                    }
                }

                {
                    Simd v = a;
                    v >>= sh;

                    for (size_t i = 0; i < N; ++i) {
                        _Type_ scalar = 0;

                        if (sh < sizeof(_Type_) * 8) {
                            scalar = _Type_(arrA[i]) >> sh;
                        }
                        else {
                            if constexpr (std::is_signed_v<_Type_>)
                                scalar = _Type_(arrA[i]) >> (sizeof(_Type_) * 8 - 1);
                        }

                        raze_assert(v[i] == scalar);
                    }
                }
            }
        }

        //{
        //    alignas(64) _Type_ arrSrc[N];
        //    std::iota(arrSrc, arrSrc + N, 50);

        //    Simd src = raze::vx::load<Simd>(arrSrc);

        //    Mask m;
        //    for (size_t i = 0; i < N; ++i)
        //        m[i] = (i % 2 == 0);

        //    auto w = raze::vx::where(a, src, m);
        //    auto wz = raze::vx::where(a, m);

        //    auto const_w = raze::vx::where(Simd(a), src, m);
        //    auto const_wz = raze::vx::where(Simd(a), m);

        //    const auto run_tests = [arrA, arrB, arrSrc, m, a, b, src](auto w, auto wz) {
        //        test_where_unary<_Type_, N>(
        //            arrA, arrSrc, m, a, src, w, wz,
        //            raze::type_traits::bit_not{},
        //            [](_Type_ A, _Type_ Src, bool cond, bool rev) {
        //                if constexpr (std::is_floating_point_v<_Type_>)
        //                    return cond ? std::bit_cast<_Type_>(~std::bit_cast<U>(A)) : Src;
        //                else
        //                    return cond ? (~A) : Src;
        //            });

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src, w, wz,
        //            raze::type_traits::bit_and{},
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                if constexpr (std::is_floating_point_v<_Type_>)
        //                    return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
        //                        & std::bit_cast<U>(A))) : Src;
        //                else
        //                    return cond ? (A & B) : Src;
        //            }
        //        );

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src, w, wz,
        //            raze::type_traits::bit_or{},
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                if constexpr (std::is_floating_point_v<_Type_>)
        //                    return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
        //                        | std::bit_cast<U>(A))) : Src;
        //                else
        //                    return cond ? (A | B) : Src;
        //            }
        //        );

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src, w, wz,
        //            raze::type_traits::bit_xor{},
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                if constexpr (std::is_floating_point_v<_Type_>)
        //                    return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
        //                        ^ std::bit_cast<U>(A))) : Src;
        //                else
        //                    return cond ? (A ^ B) : Src;
        //            }
        //        );
        //    };

        //    run_tests(w, wz);
        //    run_tests(const_w, const_wz);
        //}
    }
};

int main() {
    test_all<bitwise_tests>();
    return 0;
}
