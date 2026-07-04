#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32       _Width_>
struct slide_merge_tests {
    template <raze::sizetype _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr1[N];
        alignas(64) _Type_ arr2[N];

        std::iota(arr1, arr1 + N, 1);
        std::iota(arr2, arr2 + N, _Type_(N + 1));

        Simd v1 = raze::vx::load<Simd>(arr1);
        Simd v2 = raze::vx::load<Simd>(arr2);

        for (size_t sh = 0; sh < N; ++sh) {
            auto r1 = raze::vx::slide_left_merge(v1, v2, sh);
            auto r2 = raze::vx::slide_right_merge(v1, v2, sh);

            for (size_t i = 0; i < N; ++i) {
                raze_assert(r1[i] == ((i + sh < N)
                    ? _Type_(arr1[i + sh])
                    : _Type_(arr2[i + sh - N])));

                raze_assert(r2[i] == ((i >= sh)
                    ? _Type_(arr1[i - sh])
                    : _Type_(arr2[N - sh + i])));
            }
        }

        [&] <raze::sizetype ... I> (std::integer_sequence<raze::sizetype, I...>) {
            (([&] {
                constexpr auto __i = I;

                if constexpr (N > 16 && (I != N || I != N / 2 || I != 0 || I > 8)) {}
                else {
                    auto r1 = raze::vx::slide_left_merge(v1, v2,
                        std::integral_constant<raze::sizetype, I>{});
                    auto r2 = raze::vx::slide_right_merge(v1, v2,
                        std::integral_constant<raze::sizetype, I>{});

                    for (size_t i = 0; i < N; ++i) {
                        raze_assert(r1[i] == ((i + I < N)
                            ? _Type_(arr1[i + I])
                            : _Type_(arr2[i + I - N])));

                        raze_assert(r2[i] == ((i >= I)
                            ? _Type_(arr1[i - I])
                            : _Type_(arr2[N - I + i])));
                    }
                }
            }()), ...);
        }(std::make_integer_sequence<raze::sizetype, N>{});
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        // test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        // test_size<1>();
    }
};

int main() {
    test_all<slide_merge_tests>();
    return 0;
}