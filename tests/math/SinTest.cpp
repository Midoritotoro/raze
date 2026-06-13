#include <raze/vx/Simd.h>
#include <raze/math/Math.h>
#include <tests/vx/SimdTestTools.h>
#include <cmath>
#include <limits>

template<class T>
bool approx_equal(T a, T b, T epsilon = T(1e-5)) {
    if (std::isnan(a) && std::isnan(b)) return true;
    if (std::isinf(a) && std::isinf(b)) return (a > 0) == (b > 0);
    return raze::math::abs(a - b) <= epsilon * std::max(T(1), std::max(raze::math::abs(a), raze::math::abs(b)));
}

template <class T, class Func, class Ref>
void test_scalar_sin_op(Func f, Ref ref) {
    auto check = [&](T a) {
        T r = f(a);
        T ex = ref(a);

        if (std::isnan(ex)) raze_assert(std::isnan(r));
        else raze_assert(approx_equal(r, ex, T(1e-4)));
    };

    check(T(0));
    check(T(M_PI / 6));  // sin(30°) = 0.5
    check(T(M_PI / 4));  // sin(45°)
    check(T(M_PI / 2));  // sin(90°) = 1
    check(T(M_PI));  // sin(180°) ≈ 0
    check(T(3 * M_PI / 2));  // sin(270°) = -1
    check(T(2 * M_PI));  // sin(360°) ≈ 0

    check(T(-M_PI / 6));
    check(T(-M_PI / 2));
    check(T(-M_PI));

    check(T(0.001));
    check(T(-0.001));

    check(T(10));
    check(T(-10));
    check(T(100));
    check(T(-100));

    check(std::numeric_limits<T>::infinity());
    check(-std::numeric_limits<T>::infinity());
    check(std::numeric_limits<T>::quiet_NaN());
    check(std::numeric_limits<T>::signaling_NaN());
}

template<class T>
void test_scalar_sin_all() {
    test_scalar_sin_op<T>(
        [](T a) { return raze::math::sin(a); },
        [](T a) { return std::sin(a); });
}

template <class _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct sin_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arrIn[N], arrOut[N], arrRef[N];

        for (size_t i = 0; i < N; ++i) {
            arrIn[i] = _Type_(i * M_PI / N);
        }

        Simd input = raze::vx::load<Simd>(arrIn);
        Simd result = raze::math::sin(input);
        raze::vx::store(arrOut, result);

        for (size_t i = 0; i < N; ++i) {
            arrRef[i] = std::sin(arrIn[i]);
        }

        for (size_t i = 0; i < N; ++i) {
            raze_assert(approx_equal(arrOut[i], arrRef[i], _Type_(1e-4)));
        }

        for (size_t i = 0; i < N; ++i) {
            arrIn[i] = _Type_((i - N / 2) * 0.5);
        }

        input = raze::vx::load<Simd>(arrIn);
        result = raze::math::sin(input);
        raze::vx::store(arrOut, result);

        for (size_t i = 0; i < N; ++i) {
            arrRef[i] = std::sin(arrIn[i]);
        }

        for (size_t i = 0; i < N; ++i) {
            raze_assert(approx_equal(arrOut[i], arrRef[i], _Type_(1e-4)));
        }

        for (size_t i = 0; i < N; ++i) {
            arrIn[i] = _Type_(i * 10);
        }

        input = raze::vx::load<Simd>(arrIn);
        result = raze::math::sin(input);
        raze::vx::store(arrOut, result);

        for (size_t i = 0; i < N; ++i) {
            arrRef[i] = std::sin(arrIn[i]);
        }

        for (size_t i = 0; i < N; ++i) {
            raze_assert(approx_equal(arrOut[i], arrRef[i], _Type_(1e-3)));
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();
    }
};

int main() {
    test_scalar_sin_all<float>();
    test_scalar_sin_all<double>();

    test_all_fp<sin_tests>();
    return 0;
}