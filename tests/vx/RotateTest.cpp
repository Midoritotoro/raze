#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct rotate_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N];
        std::iota(arr, arr + N, 1);

        Simd v = raze::vx::load<Simd>(arr);

        for (size_t sh = 0; sh < N; ++sh) {
            auto rl = raze::vx::rotate_left(v, sh);
            auto rr = raze::vx::rotate_right(v, sh);

            for (size_t i = 0; i < N; ++i) {
                raze_assert(rl[i] == arr[(i + sh) % N]);
                raze_assert(rr[i] == arr[(i + N - sh) % N]);
            }
        }

        [&] <size_t... I>(std::index_sequence<I...>) {
            (([&] {
                auto rl = raze::vx::rotate_left(v, std::integral_constant<size_t, I>{});
                auto rr = raze::vx::rotate_right(v, std::integral_constant<size_t, I>{});

                for (size_t i = 0; i < N; ++i) {
                    raze_assert(rl[i] == _Type_(arr[(i + I) % N]));
                    raze_assert(rr[i] == _Type_(arr[(i + N - I) % N]));
                }
            }()), ...);
        }(std::make_index_sequence<N>{});
    }
};

int main() {
    test_all<rotate_tests>();
    return 0;
}
