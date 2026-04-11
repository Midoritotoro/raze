#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct select_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ a_arr[N], b_arr[N];
        for (size_t i = 0; i < N; ++i) {
            a_arr[i] = _Type_(i + 1);
            b_arr[i] = _Type_(200 + i);
        }

        Simd a = raze::vx::load<Simd>(a_arr);
        Simd b = raze::vx::load<Simd>(b_arr);

        auto test_mask = [&](Mask mask) {
            auto r = raze::vx::select(a, b, mask);

            for (size_t i = 0; i < N; ++i) {
                _Type_ expected = mask[i] ? a_arr[i] : b_arr[i];
                raze_assert(r[i] == expected);
            }
        };

        test_mask(Mask(true));
        test_mask(Mask(false));
        test_mask(make_alternating_mask<Mask>());

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i)
            test_mask(make_random_mask<Mask>());
    }
};

int main() {
    test_all<select_tests>();
    return 0;
}
