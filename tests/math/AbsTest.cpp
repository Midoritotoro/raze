#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Simd.h>
#include <raze/math/Math.h>
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

template <class  _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct abs_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;

        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N], fallback[N];
        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i) {
            if constexpr (std::is_signed_v<_Type_>)
                arr[i] = _Type_((i % 2 == 0) ? -(_Type_(i) + 1) : (_Type_(i) + 1));
            else
                arr[i] = _Type_(i + 1);
        }
    
        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        {
            auto r = raze::math::abs(v);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == (arr[i] < _Type_(0) ? _Type_(-arr[i]) : _Type_(arr[i])));
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto r = raze::math::abs[m, fbk](v);

                for (size_t i = 0; i < N; ++i)
                    raze_assert(r[i] == _Type_(m[i] ? raze::math::abs(arr[i]) : fallback[i]));
            }

            {
                auto r = raze::math::abs[m](v);

                for (size_t i = 0; i < N; ++i)
                    raze_assert(r[i] == _Type_(m[i] ? raze::math::abs(arr[i]) : 0));
            }
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<1>();
    }
};

int main() {
    test_scalar_abs_type<int8_t>();
    test_scalar_abs_type<int16_t>();
    test_scalar_abs_type<int32_t>();
    test_scalar_abs_type<int64_t>();

    test_scalar_abs_type<uint8_t>();
    test_scalar_abs_type<uint16_t>();
    test_scalar_abs_type<uint32_t>();
    test_scalar_abs_type<uint64_t>();

    test_scalar_abs_type<float>();
    test_scalar_abs_type<double>();

    test_all<abs_tests>();
    return 0;
}
