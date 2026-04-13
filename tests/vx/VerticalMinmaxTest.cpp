#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct vertical_minmax_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N], fallback[N];
        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_((i % 2 == 0) ? (i + 1) : -(i + 3));
            arrB[i] = _Type_((i % 3 == 0) ? (i + 5) : -(i + 2));
        }

        Simd a = raze::vx::load<Simd>(arrA);
        Simd b = raze::vx::load<Simd>(arrB);
        Simd fbk = raze::vx::load<Simd>(fallback);

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

            auto wz = raze::vx::where(a, m);
            auto w = raze::vx::where(a, fbk, m);

            auto const_wz = raze::vx::where(Simd(a), m);
            auto const_w = raze::vx::where(Simd(a), fbk, m);


            {
                auto wz_min = raze::vx::vertical_min(b, wz);
                auto wz_max = raze::vx::vertical_max(b, wz);

                auto constwz_min = raze::vx::vertical_min(b, const_wz);
                auto constwz_max = raze::vx::vertical_max(b, const_wz);
                
                raze_assert(raze::vx::all_of(constwz_min == wz_min));
                raze_assert(raze::vx::all_of(constwz_max == wz_max));

                for (size_t i = 0; i < N; ++i) {
                    if (m[i])
                        raze_assert(wz_min[i] == std::min(arrB[i], arrA[i]) &&
                            wz_max[i] == std::max(arrB[i], arrA[i]));
                    else
                        raze_assert(wz_min[i] == _Type_(0) && wz_max[i] == _Type_(0));
                }
            }

            {
                auto w_min = raze::vx::vertical_min(w, b);
                auto w_max = raze::vx::vertical_max(w, b);

                auto constw_min = raze::vx::vertical_min(b, const_w);
                auto constw_max = raze::vx::vertical_max(b, const_w);

                raze_assert(raze::vx::all_of(constw_min == w_min));
                raze_assert(raze::vx::all_of(constw_max == w_max));

                for (size_t i = 0; i < N; ++i) {
                    if (m[i])
                        raze_assert(w_min[i] == std::min(arrB[i], arrA[i]) &&
                            w_max[i] == std::max(arrB[i], arrA[i]));
                    else
                        raze_assert(w_min[i] == fallback[i] && w_max[i] == fallback[i]);
                }
            }
        }
    }
};

int main() {
    test_all<vertical_minmax_tests>();
    return 0;
}
