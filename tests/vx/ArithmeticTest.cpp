#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct arithmetic_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    static constexpr size_t N = Simd::size();

    template<class SimdOp, class ScalarOp>
    static void run(const Simd& a, const Simd& b,
        const _Type_(&A)[N], const _Type_(&B)[N],
        SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == _Type_(sc(A[i], B[i])));
    }

    template<class SimdOp, class ScalarOp>
    static void run_unary(const Simd& a,
        const _Type_(&A)[N],
        SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a);
        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == _Type_(sc(A[i])));
    }

    template<class SimdOp, class ScalarOp>
    static void run_assign(const Simd& a0, const Simd& b,
        const _Type_(&A)[N], const _Type_(&B)[N],
        SimdOp sop, ScalarOp sc)
    {
        Simd a = a0;
        sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(a[i] == _Type_(sc(A[i], B[i])));
    }

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N];

        std::iota(arrA, arrA + N, _Type_(1));
        std::iota(arrB, arrB + N, _Type_(1));

        std::transform(arrB, arrB + N, arrB, [](auto x) { return x * 3; });

        Simd a = raze::vx::load<Simd>(arrA);
        Simd b = raze::vx::load<Simd>(arrB);

        run(a, b, arrA, arrB, [](auto x, auto y) { return x + y; }, [](auto x, auto y) { return x + y; });
        run(a, b, arrA, arrB, [](auto x, auto y) { return x - y; }, [](auto x, auto y) { return x - y; });
        run(a, b, arrA, arrB, [](auto x, auto y) { return x * y; }, [](auto x, auto y) { return x * y; });

        std::iota(arrB, arrB + N, _Type_(1));
        b = raze::vx::load<Simd>(arrB);

        run(a, b, arrA, arrB, [](auto x, auto y) { return x / y; }, [](auto x, auto y) { return x / y; });

        run_unary(a, arrA, [](auto x) { return -x; }, [](auto x) { return -x; });

        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x += y; }, [](auto x, auto y) { return x + y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x -= y; }, [](auto x, auto y) { return x - y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x *= y; }, [](auto x, auto y) { return x * y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y) { x /= y; }, [](auto x, auto y) { return x / y; });

       /* {
            alignas(64) _Type_ arrSrc[N];
            std::iota(arrSrc, arrSrc + N, 50);

            Simd src = raze::vx::load<Simd>(arrSrc);

            Mask m;
            for (size_t i = 0; i < N; ++i)
                m[i] = (i % 2 == 0);

            auto w = raze::vx::where(a, src, m);
            auto wz = raze::vx::where(a, m);

            auto const_w = raze::vx::where(Simd(a), src, m);
            auto const_wz = raze::vx::where(Simd(a), m);
            
            const auto run_tests = [arrA, arrB, arrSrc, a, b, m, src](auto w, auto wz) {
                test_where_unary<_Type_, N>(
                    arrA, arrSrc, m, a, src, w, wz,
                    raze::type_traits::negate{},
                    [](_Type_ A, _Type_ Src, bool cond, bool rev) {
                        return cond ? (-A) : Src;
                    });

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src, w, wz,
                    raze::type_traits::plus{},
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        return cond ? (rev ? B + A : A + B) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src, w, wz,
                    raze::type_traits::minus{},
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        return cond ? (rev ? B - A : A - B) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src, w, wz,
                    raze::type_traits::multiplies{},
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        return cond ? (rev ? B * A : A * B) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src, w, wz,
                    raze::type_traits::divides{},
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        return cond ? (rev ? B / A : A / B) : Src;
                    }
                );
            };

            run_tests(w, wz);
            run_tests(const_w, const_wz);
        }*/
    }
};

int main() {
    test_all<arithmetic_tests>();
    return 0;
}
