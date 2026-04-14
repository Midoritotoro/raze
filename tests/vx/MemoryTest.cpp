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

            Simd va; va.copy_from(arr, raze::vx::aligned);
            Simd vu; vu.copy_from(arr);

            alignas(64) _Type_ out[N];
            _Type_ outU[N];

            va.copy_to(out, raze::vx::aligned);
            vu.copy_to(outU);

            raze_assert(std::equal(out, out + N, outU, outU + N));
            raze_assert(std::equal(out, out + N, arr, arr + N));
        }

        {
            alignas(64) _Type_ src[N], fallback[N], dst[N];

            for (size_t i = 0; i < N; ++i) {
                src[i] = _Type_(i + 1);
                fallback[i] = _Type_(100 + i);
                dst[i] = _Type_(200 + i);
            }

            const auto test_all = [&] (Mask mask) {
                {
                    Simd vu;
                    vu.copy_from(src, mask);

                    Simd va;
                    va.copy_from(src, mask, raze::vx::aligned);

                    raze_assert(raze::vx::all_of(va == vu));

                    for (size_t i = 0; i < N; ++i)
                        raze_assert(vu[i] == (mask[i] ? src[i] : _Type_(0)));
                }

                {
                    Simd fb;
                    fb.copy_from(fallback);

                    Simd vu; 
                    vu.copy_from(src, mask, fb);

                    Simd va;
                    va.copy_from(src, mask, fb, raze::vx::aligned);

                    raze_assert(raze::vx::all_of(va == vu));

                    for (size_t i = 0; i < N; ++i)
                        raze_assert(vu[i] == (mask[i] ? src[i] : fallback[i]));
                }

                {
                    Simd val(77);
                    val.copy_to(dst, mask);

                    for (size_t i = 0; i < N; ++i)
                        raze_assert(dst[i] == (mask[i] ? _Type_(77) : _Type_(200 + i)));
                }
            };
            
            for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
                test_all(make_random_mask<Mask>());
                test_all(make_random_mask<Mask>());
            }
        }
    }
};

int main() {
    test_all<memory_tests>();
    return 0;
}
