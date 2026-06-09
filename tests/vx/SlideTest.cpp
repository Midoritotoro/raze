#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct slide_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N];
        std::iota(arr, arr + N, 1);

        Simd v = raze::vx::load<Simd>(arr);

        //for (size_t sh = 0; sh < N; ++sh) {
        //    auto r1 = raze::vx::slide_left(v, sh);
        //    auto r2 = raze::vx::slide_right(v, sh);

        //    for (size_t i = 0; i < N; ++i) {
        //        raze_assert(r1[i] == ((i + sh < N) ? _Type_(arr[i + sh]) : _Type_(0)));
        //        raze_assert(r2[i] == ((i >= sh) ? _Type_(arr[i - sh]) : _Type_(0)));
        //    }
        //}

        [&] <raze::sizetype ... I>(std::integer_sequence<raze::sizetype, I...>) {
            (([&] {
                if constexpr (N > 16 && (I != N || I != N / 2 || I != 0 || I > 8)) {}
                else {
                    auto r1 = raze::vx::slide_left(v, std::integral_constant<raze::sizetype, I>{});
                    auto r2 = raze::vx::slide_right(v, std::integral_constant<raze::sizetype, I>{});

                    for (size_t i = 0; i < N; ++i) {
                        raze_assert(r1[i] == ((i + I < N) ? _Type_(arr[i + I]) : _Type_(0)));
                        raze_assert(r2[i] == ((i >= I) ? _Type_(arr[i - I]) : _Type_(0)));
                    }
                }
            }()), ...);
        }(std::make_integer_sequence<raze::sizetype, N + 1>{});
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        test_size<1>();
    }
};

int main() {
    test_all<slide_tests>();
    return 0;
}
