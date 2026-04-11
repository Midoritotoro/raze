#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>


template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct abs_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N], fallback[N];
        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i) {
            if constexpr (std::is_signed_v<_Type_>)
                arr[i] = _Type_((i % 2 == 0) ? -(_Type_(i) + 1) : (_Type_(i) + 1));
            else
                arr[i] = _Type_(i + 1);
        }
    
        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        {
            auto r = raze::math::abs(v);

            for (size_t i = 0; i < N; ++i)
                raze_assert(r[i] == (arr[i] < _Type_(0) ? _Type_(-arr[i]) : _Type_(arr[i])));
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            auto w = raze::vx::where(v, fbk, m);
            auto wz = raze::vx::where(v, m);

            {
                auto r = raze::math::abs(w);

                for (size_t i = 0; i < N; ++i)
                    raze_assert(r[i] == _Type_(m[i] ? raze::math::abs(arr[i]) : fallback[i]));
            }

            {
                auto r = raze::math::abs(wz);

                for (size_t i = 0; i < N; ++i)
                    raze_assert(r[i] == _Type_(m[i] ? raze::math::abs(arr[i]) : 0));
            }
        }
    }
};

int main() {
    test_all<abs_tests>();
    return 0;
}
