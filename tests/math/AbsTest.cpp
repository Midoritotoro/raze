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

    auto check_cond = [](T x, T expected, bool cond) {
        T r = raze::math::abs[cond](x);
        raze_assert(r == expected);
    };

    auto check_cond_fbk = [](T x, T expected, bool cond, T fbk) {
        T r = raze::math::abs[cond, fbk](x);
        raze_assert(r == expected);
    };

    std::mt19937_64 rng(0x123456789ABCDEFULL);

    if constexpr (std::is_signed_v<T>) {
        for (auto i = 0; i < 1000; ++i) {
            const auto num = T(rng());
            check(num, std::abs(num));
        }

        for (auto i = 0; i < 1000; ++i) {
            const auto num = T(rng());
            const auto fbk = T(rng());
            const bool cond = (i % 2) == 0;

            check_cond_fbk(num, cond ? std::abs(num) : fbk, cond, fbk);
        }

        for (auto i = 0; i < 1000; ++i) {
            const auto num = T(rng());
            const bool cond = (i % 2) == 0;

            check_cond_fbk(num, cond ? std::abs(num) : 0, cond, 0);
        }
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
