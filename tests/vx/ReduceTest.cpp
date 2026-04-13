#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct reduce_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N], fallback[N];
        std::iota(arr, arr + N, 1);
        std::iota(fallback, fallback + N, 42);

        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        raze_assert(raze::vx::reduce_add(v) == std::accumulate(arr, arr + N, 
            raze::vx::__reduce_type<_Type_>{0}, std::plus{}));

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            auto w = raze::vx::where(v, fbk, m);
            auto wz = raze::vx::where(v, m);

            auto const_w = raze::vx::where(Simd(v), fbk, m);
            auto const_wz = raze::vx::where(Simd(v), m);

            {
                auto r1 = raze::vx::reduce_add(w);
                auto r2 = raze::vx::reduce_add(const_w);

                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i])
                        expected += arr[i];
                    else
                        expected += fallback[i];

                raze_assert(expected == r1);
                raze_assert(expected == r2);
            }

            {
                auto r1 = raze::vx::reduce_add(wz);
                auto r2 = raze::vx::reduce_add(const_wz);

                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i])
                        expected += arr[i];

                raze_assert(expected == r1);
                raze_assert(expected == r2);
            }
        }
    }
};

int main() {
    test_all<reduce_tests>();
    return 0;
}
