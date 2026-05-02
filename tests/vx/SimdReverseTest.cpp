#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct reverse_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N];

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_(i + 1);

        Simd v = raze::vx::load<Simd>(arr);
        auto r = raze::vx::reverse(v);

        for (size_t i = 0; i < N; ++i)
            raze_assert(r[i] == arr[N - 1 - i]);
    }
};

int main() {
    test_all<reverse_tests>();
    return 0;
}
