#include <raze/math/Math.h>
#include <tests/vx/SimdTestTools.h>
#include <limits>
#include <type_traits>

template<class T>
void test_scalar_abs_type() {
    auto check = [](T x, T expected) {
        T r = raze::math::abs(x);
        raze_assert(r == expected);
    };

    if constexpr (std::is_signed_v<T>) {
        check(T(0), T(0));
        check(T(1), T(1));
        check(T(-1), T(1));

        check(T(5), T(5));
        check(T(-5), T(5));
        check(T(123), T(123));
        check(T(-123), T(123));
    }

    if constexpr (std::is_signed_v<T>) {
        check(std::numeric_limits<T>::max(),
            std::numeric_limits<T>::max());

        T mn = std::numeric_limits<T>::min();
        T expected = (mn < 0 ? T(-(mn + 1)) + 1 : mn);

        check(mn, expected);
    }
    else {
        check(std::numeric_limits<T>::max(),
            std::numeric_limits<T>::max());
    }
}

int main() {
    test_scalar_abs_type<int8_t>();
    test_scalar_abs_type<int16_t>();
    test_scalar_abs_type<int32_t>();
    test_scalar_abs_type<int64_t>();

    test_scalar_abs_type<uint8_t>();
    test_scalar_abs_type<uint16_t>();
    test_scalar_abs_type<uint32_t>();
    test_scalar_abs_type<uint64_t>();

    return 0;
}
