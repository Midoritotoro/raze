#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct reduce_tests {
    template <raze::u64 _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N], fallback[N];
        std::iota(arr, arr + N, 1);
        std::iota(fallback, fallback + N, 42);

        Simd v;
        v.copy_from(arr);

        Simd fbk;
        fbk.copy_from(fallback);

        {
            auto result = raze::vx::hsum(v);
            auto expected = std::accumulate(arr, arr + N, raze::vx::__reduce_type<_Type_>{0}, std::plus{});
            raze_assert(result == expected);
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto r1 = raze::vx::hsum[m, fbk](v);
                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i]) expected += arr[i];
                    else expected += fallback[i];

                raze_assert(expected == r1);
            }

            {
                auto r1 = raze::vx::hsum[m](v);
                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i]) expected += arr[i];

                raze_assert(expected == r1);
            }
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<1>();
        test_size<7>();
    }
};

int main() {
    test_all<reduce_tests>();
    return 0;
}
