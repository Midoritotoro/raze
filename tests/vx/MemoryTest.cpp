#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct memory_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        {
            alignas(64) _Type_ arr[N];
            for (size_t i = 0; i < N; ++i)
                arr[i] = _Type_(i + 1);

            Simd va;
            va = Simd::copy_from[raze::vx::aligned](arr);
            Simd vu;
            vu = Simd::copy_from(arr);



            alignas(64) _Type_ out[N];
            _Type_ outU[N];

           /* raze::vx::store(out, va);
            raze::vx::store(outU, vu);

            for (size_t i = 0; i < N; ++i)
                raze_assert(out[i] == arr[i]);*/

  
        }

       /* {
            alignas(64) _Type_ src[N], fallback[N], dst[N];

            for (size_t i = 0; i < N; ++i) {
                src[i] = _Type_(i + 1);
                fallback[i] = _Type_(100 + i);
                dst[i] = _Type_(200 + i);
            }

            const auto test_all = [&]<class _AlignmentPolicy_>(_AlignmentPolicy_ && policy, Mask mask) {
                {
                    Simd v = raze::vx::load(raze::vx::where(src, mask), policy);
                    for (size_t i = 0; i < N; ++i)
                        raze_assert(v[i] == (mask[i] ? src[i] : T(0)));
                }

                {
                    Simd fb = raze::vx::load<Simd>(fallback);
                    Simd v = raze::vx::load(raze::vx::where(src, fb, mask), policy);

                    for (size_t i = 0; i < N; ++i)
                        raze_assert(v[i] == (mask[i] ? src[i] : fallback[i]));
                }

                {
                    Simd val(77);
                    raze::vx::store(dst, raze::vx::where(val, mask), policy);

                    for (size_t i = 0; i < N; ++i)
                        raze_assert(dst[i] == (mask[i] ? _Type_(77) : _Type_(200 + i)));
                }
            };
            
            for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
                test_all(raze::vx::aligned_policy(), make_random_mask<Mask>());
                test_all(raze::vx::unaligned_policy(), make_random_mask<Mask>());
            }
        }*/
    }
};

int main() {
    test_all<memory_tests>();
    return 0;
}
