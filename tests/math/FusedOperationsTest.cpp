#include <raze/vx/Simd.h>
#include <raze/math/Math.h>
#include <tests/vx/SimdTestTools.h>
#include <cmath>
#include <limits>

template<class T, class Func, class Ref>
void test_scalar_fma_op(Func f, Ref ref) {
    auto check = [&](T a, T b, T c) {
        T r  = f(a, b, c);
        T ex = ref(a, b, c);

        if constexpr (std::is_floating_point_v<T>) {
            if (std::isnan(ex))
                raze_assert(std::isnan(r));
            else
                raze_assert(r == ex);
        } else {
            raze_assert(r == ex);
        }
    };

    check(T(1), T(2), T(3));
    check(T(-1), T(2), T(3));
    check(T(1), T(-2), T(3));
    check(T(1), T(2), T(-3));

    check(T(0), T(5), T(7));
    check(T(5), T(0), T(7));
    check(T(5), T(7), T(0));

    check(T(-3), T(-4), T(5));
    check(T(3), T(-4), T(-5));

    check(T(1000), T(2000), T(3000));
    check(T(-1000), T(2000), T(-3000));

    if constexpr (std::is_floating_point_v<T>) {
        check(T(1), T(2), std::numeric_limits<T>::infinity());
        check(std::numeric_limits<T>::infinity(), T(2), T(3));

        check(std::numeric_limits<T>::quiet_NaN(), T(2), T(3));
    }
}

template<class T>
void test_scalar_fma_all() {
    test_scalar_fma_op<T>(
        [](T a, T b, T c){ return raze::math::fma(a,b,c); },
        [](T a, T b, T c){ return T(a*b + c); });

    test_scalar_fma_op<T>(
        [](T a, T b, T c){ return raze::math::fms(a,b,c); },
        [](T a, T b, T c){ return T(a*b - c); });

    test_scalar_fma_op<T>(
        [](T a, T b, T c){ return raze::math::fnms(a,b,c); },
        [](T a, T b, T c){ return T(-(a*b) - c); });

    test_scalar_fma_op<T>(
        [](T a, T b, T c){ return raze::math::fnma(a,b,c); },
        [](T a, T b, T c){ return T(-(a*b) + c); });
}

template <class _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct fused_operations_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arrA[N], arrB[N], arrC[N], arrSrc[N];

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_(i + 1);
            arrB[i] = _Type_((i + 1) * 3);
            arrC[i] = _Type_((i + 1) * 7);
            arrSrc[i] = _Type_(100 + i);
        }

        Simd a = raze::vx::load<Simd>(arrA);
        Simd b = raze::vx::load<Simd>(arrB);
        Simd c = raze::vx::load<Simd>(arrC);
        Simd src = raze::vx::load<Simd>(arrSrc);

        Mask m;
        for (size_t i = 0; i < N; ++i)
            m[i] = (i % 2 == 0);

        test_where_ternary<_Type_, N>(arrA, arrB, arrC, arrSrc, m, a, b, c, src,
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fma[m, src](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fma[m](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z, _Type_ src_scalar, bool cond) { return raze::math::fma[cond, src_scalar](x, y, z); });
            
        test_where_ternary<_Type_, N>(arrA, arrB, arrC, arrSrc, m, a, b, c, src,
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fms[m, src](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fms[m](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z, _Type_ src_scalar, bool cond) { return raze::math::fms[cond, src_scalar](x, y, z); });

        test_where_ternary<_Type_, N>(arrA, arrB, arrC, arrSrc, m, a, b, c, src,
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnma[m, src](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnma[m](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z, _Type_ src_scalar, bool cond) { return raze::math::fnma[cond, src_scalar](x, y, z); });

        test_where_ternary<_Type_, N>(arrA, arrB, arrC, arrSrc, m, a, b, c, src,
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnms[m, src](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnms[m](x, y, z); },
            [&](const auto& x, const auto& y, const auto& z, _Type_ src_scalar, bool cond) { return raze::math::fnms[cond, src_scalar](x, y, z); });
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();

    }
};

int main() {
    test_scalar_fma_all<float>();
    test_scalar_fma_all<double>();
    test_scalar_fma_all<long long>();
    test_scalar_fma_all<unsigned long long>();
    test_scalar_fma_all<long>();
    test_scalar_fma_all<unsigned long>();
    test_scalar_fma_all<int>();
    test_scalar_fma_all<unsigned int>();
    test_scalar_fma_all<short>();
    test_scalar_fma_all<unsigned short>();
    test_scalar_fma_all<char>();
    test_scalar_fma_all<unsigned char>();

    test_all<fused_operations_tests>();
    return 0;
}
