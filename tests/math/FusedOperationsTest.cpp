#include <raze/math/Math.h>
#include <tests/vx/SimdTestTools.h>
#include <cmath>
#include <limits>

template<class T, class Func, class Ref>
void test_scalar_fma_op(const char* name, Func f, Ref ref) {
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
    using raze::math::fma;
    using raze::math::fms;
    using raze::math::fnms;
    using raze::math::fnma;

    test_scalar_fma_op<T>("fma",
        [](T a, T b, T c){ return fma(a,b,c); },
        [](T a, T b, T c){ return T(a*b + c); });

    test_scalar_fma_op<T>("fms",
        [](T a, T b, T c){ return fms(a,b,c); },
        [](T a, T b, T c){ return T(a*b - c); });

    test_scalar_fma_op<T>("fnms",
        [](T a, T b, T c){ return fnms(a,b,c); },
        [](T a, T b, T c){ return T(-(a*b) - c); });

    test_scalar_fma_op<T>("fnma",
        [](T a, T b, T c){ return fnma(a,b,c); },
        [](T a, T b, T c){ return T(-(a*b) + c); });
}

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

    return 0;
}
