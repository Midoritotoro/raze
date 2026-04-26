#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct arithmetic_tests {
    template<raze::sizetype N, class Simd, class SimdOp, class ScalarOp>
    static void run(const Simd& a, const Simd& b,
        const _Type_(&A)[N], const _Type_(&B)[N],
        SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == _Type_(sc(A[i], B[i])));
    }

    template<raze::sizetype N, class Simd, class SimdOp, class ScalarOp>
    static void run_unary(const Simd& a,
        const _Type_(&A)[N],
        SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a);
        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == _Type_(sc(A[i])));
    }

    template<raze::sizetype N, class Simd, class SimdOp, class ScalarOp>
    static void run_assign(const Simd& a0, const Simd& b,
        const _Type_(&A)[N], const _Type_(&B)[N],
        SimdOp sop, ScalarOp sc)
    {
        Simd a = a0;
        sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(a[i] == _Type_(sc(A[i], B[i])));
    }

    template <raze::sizetype N>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, N>>;
        using Mask = typename Simd::mask_type;

        alignas(64) _Type_ arrA[N], arrB[N];

        std::iota(arrA, arrA + N, _Type_(1));
        std::iota(arrB, arrB + N, _Type_(1));

        std::transform(arrB, arrB + N, arrB, [](auto x) { return x * 3; });

        Simd a; a.copy_from(arrA);
        Simd b; b.copy_from(arrB);

        run(a, b, arrA, arrB, [](auto x, auto y) { return x + y; }, [](auto x, auto y) { return x + y; });
        run(a, b, arrA, arrB, [](auto x, auto y) { return x - y; }, [](auto x, auto y) { return x - y; });
        run(a, b, arrA, arrB, [](auto x, auto y) { return x * y; }, [](auto x, auto y) { return x * y; });
        
        std::iota(arrB, arrB + N, _Type_(1));
        b.copy_from(arrB); 

        run(a, b, arrA, arrB, [](auto x, auto y) { return x / y; }, [](auto x, auto y) { return x / y; });
        
        run_unary(a, arrA, [](auto x) { return -x; }, [](auto x) { return -x; });
        
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x += y; }, [](auto x, auto y) { return x + y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x -= y; }, [](auto x, auto y) { return x - y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x *= y; }, [](auto x, auto y) { return x * y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x /= y; }, [](auto x, auto y) { return x / y; });

        //{
        //    alignas(64) _Type_ arrSrc[N];
        //    std::iota(arrSrc, arrSrc + N, 50);

        //    Simd src; src.copy_from(arrSrc);

        //    const auto run_tests = [arrA, arrB, arrSrc, a, b, src](auto m) {
        //        test_where_unary<_Type_, N>(
        //            arrA, arrSrc, m, a, src,
        //            [m, src](Simd A) {
        //                return raze::vx::neg[m, src](A);
        //            },
        //            [m](Simd A) {
        //                return raze::vx::neg[m](A);
        //            },
        //            [](_Type_ A, _Type_ Src, bool cond, bool rev) {
        //                return cond ? (-A) : Src;
        //        });

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src,
        //            [m, src](Simd A, Simd B) {
        //                return raze::vx::add[m, src](A, B);
        //            },
        //            [m](Simd A, Simd B) {
        //                return raze::vx::add[m](A, B);
        //            },
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                return cond ? (rev ? B + A : A + B) : Src;
        //            }
        //        );

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src,
        //            [m, src](Simd A, Simd B) {
        //                return raze::vx::sub[m, src](A, B);
        //            },
        //            [m](Simd A, Simd B) {
        //                return raze::vx::sub[m](A, B);
        //            },
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                return cond ? (rev ? B - A : A - B) : Src;
        //            }
        //        );

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src,
        //            [m, src](Simd A, Simd B) {
        //                return raze::vx::mul[m, src](A, B);
        //            },
        //            [m](Simd A, Simd B) {
        //                return raze::vx::mul[m](A, B);
        //            },
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                return cond ? (rev ? B * A : A * B) : Src;
        //            }
        //        );

        //        test_where_binary<_Type_, N>(
        //            arrA, arrB, arrSrc, m,
        //            a, b, src,
        //            [m, src](Simd A, Simd B) {
        //                return raze::vx::div[m, src](A, B);
        //            },
        //            [m](Simd A, Simd B) {
        //                return raze::vx::div[m](A, B);
        //            },
        //            [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
        //                return cond ? (rev ? B / A : A / B) : Src;
        //            }
        //        );
        //    };

        //    for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
        //        run_tests(make_random_mask<Mask>());
        //    }
        //}
    }

    void operator()() {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        //test_size<1>();
        //test_size<7>();
        //test_size<17>();
        //test_size<31>();
    }
};

int main() {
    test_all<arithmetic_tests>();
    return 0;
}
