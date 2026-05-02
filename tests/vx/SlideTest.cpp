#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct slide_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N];
        std::iota(arr, arr + N, 1);

        Simd v = raze::vx::load<Simd>(arr);

        for (size_t sh = 0; sh <= N; ++sh) {
            auto r1 = raze::vx::slide_left(v, sh);
            auto r2 = raze::vx::slide_right(v, sh);

            for (size_t i = 0; i < N; ++i) {
                raze_assert(r1[i] == ((i + sh < N) ? _Type_(arr[i + sh]) : _Type_(0)));
                raze_assert(r2[i] == ((i >= sh) ? _Type_(arr[i - sh]) : _Type_(0)));
            }
        }

        [&] <size_t... I>(std::index_sequence<I...>) {
            (([&] {
                auto r1 = raze::vx::slide_left(v, std::integral_constant<size_t, I>{});
                auto r2 = raze::vx::slide_right(v, std::integral_constant<size_t, I>{});

                for (size_t i = 0; i < N; ++i) {
                    raze_assert(r1[i] == ((i + I < N) ? _Type_(arr[i + I]) : _Type_(0)));
                    raze_assert(r2[i] == ((i >= I) ? _Type_(arr[i - I]) : _Type_(0)));
                }
            }()), ...);
        }(std::make_index_sequence<N + 1>{});
    }
};

int main() {
    test_all<slide_tests>();
    return 0;
}
