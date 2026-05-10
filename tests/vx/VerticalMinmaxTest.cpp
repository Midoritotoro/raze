#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct vertical_minmax_tests {
    template <raze::sizetype N>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, N>>;
        using Mask = typename Simd::mask_type;
        using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arrA[N], arrB[N], fallback[N];
        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_((i % 2 == 0) ? (i + 1) : -(i + 3));
            arrB[i] = _Type_((i % 3 == 0) ? (i + 5) : -(i + 2));
        }

        Simd a; a.copy_from(arrA);
        Simd b;  b.copy_from(arrB);
        Simd fbk; fbk.copy_from(fallback);

        {
            auto vmin = raze::vx::vertical_min(a, b);
            auto vmax = raze::vx::vertical_max(a, b);

            for (size_t i = 0; i < N; ++i) {
                raze_assert(vmin[i] == std::min(arrA[i], arrB[i]));
                raze_assert(vmax[i] == std::max(arrA[i], arrB[i]));
            }
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto vmin = raze::vx::vertical_min[m](a, b);
                auto vmax = raze::vx::vertical_max[m](a, b);

                for (size_t i = 0; i < N; ++i) {
                    if (m[i])
                        raze_assert(vmin[i] == std::min(arrB[i], arrA[i]) &&
                            vmax[i] == std::max(arrB[i], arrA[i]));
                    else
                        raze_assert(vmin[i] == _Type_(0) && vmax[i] == _Type_(0));
                }
            }

            {
                auto vmin = raze::vx::vertical_min[m, fbk](a, b);
                auto vmax = raze::vx::vertical_max[m, fbk](a, b);

                for (size_t i = 0; i < N; ++i) {
                    if (m[i])
                        raze_assert(vmin[i] == std::min(arrB[i], arrA[i]) &&
                            vmax[i] == std::max(arrB[i], arrA[i]));
                    else
                        raze_assert(vmin[i] == fallback[i] && vmax[i] == fallback[i]);
                }
            }
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();
    }
};

int main() {
    test_all<vertical_minmax_tests>();
    return 0;
}
