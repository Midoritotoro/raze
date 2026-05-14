#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>
#include <numeric>
#include <cmath>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct variable_length_memory_tests {
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;

    template <raze::u32 _N_>
    void test_size() const {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        using Mask = raze::vx::simd_mask<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        constexpr size_t N = _N_;
        constexpr size_t PAD = 4;

        _Type_ fbk[N];
        std::iota(fbk, fbk + N, _Type_(42));
        auto fbk_v = raze::vx::load<Simd>(fbk);

        {
            alignas(64) _Type_ src[N];
            alignas(64) _Type_ dst[N];
            for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 3 + 1);

            Simd v = raze::vx::load<Simd>[raze::vx::aligned](src);
            raze::vx::store[raze::vx::aligned](dst, v);
            raze_assert(std::equal(src, src + N, dst));
        }

        {
            alignas(64) _Type_ buf[N * 2 + PAD];
            _Type_* src = buf + 1;
            _Type_* dst = buf + N + 1;
            for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 7 + 2);

            Simd v = raze::vx::load<Simd>(src);
            raze::vx::store(dst, v);
            raze_assert(std::equal(src, src + N, dst));
        }

        auto tests_for_mask = [=](auto mask, auto fallback) {
            {
                alignas(64) _Type_ src[N];
                alignas(64) _Type_ dst[N];
                for (size_t i = 0; i < N; ++i) {
                    src[i] = static_cast<_Type_>(i * 3 + 1);
                    dst[i] = static_cast<_Type_>(0xDEADBEEF);
                }

                Simd v = raze::vx::load<Simd>[mask](src);
                raze::vx::store[mask](dst, v);

                for (size_t i = 0; i < N; ++i) {
                    if (mask[i]) raze_assert(dst[i] == src[i]);
                }
            }

            {
                alignas(64) _Type_ src[N];
                alignas(64) _Type_ dst[N];
                for (size_t i = 0; i < N; ++i) {
                    src[i] = static_cast<_Type_>(i * 5 + 2);
                    dst[i] = static_cast<_Type_>(0xCAFEBABE);
                }

                Simd v = raze::vx::load<Simd>[mask][raze::vx::aligned](src);
                raze::vx::store[mask][raze::vx::aligned](dst, v);

                for (size_t i = 0; i < N; ++i) {
                    if (mask[i]) raze_assert(dst[i] == src[i]);
                }
            }

            {
                alignas(64) _Type_ buf[N * 2 + PAD];
                _Type_* src = buf + 1;
                for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 7 + 3);

                Simd v = raze::vx::load<Simd>[mask, fallback](src);

                for (size_t i = 0; i < N; ++i) {
                    _Type_ expected = mask[i] ? src[i] : fallback[i];
                    raze_assert(v[i] == expected);
                }
            }

            {
                alignas(64) _Type_ src[N];
                for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 11 + 4);

                Simd v = raze::vx::load<Simd>[mask, fallback][raze::vx::aligned](src);

                for (size_t i = 0; i < N; ++i) {
                    _Type_ expected = mask[i] ? src[i] : fallback[i];
                    raze_assert(v[i] == expected);
                }
            }

            {
                alignas(64) _Type_ buf[N * 2 + PAD];
                _Type_* dst = buf + 1;
                for (size_t i = 0; i < N; ++i) dst[i] = static_cast<_Type_>(0x12345678);

                Simd val;
                for (size_t i = 0; i < N; ++i) {
                    alignas(64) _Type_ tmp[N];
                    for (size_t j = 0; j < N; ++j) tmp[j] = static_cast<_Type_>(j * 13 + i);
                    val = raze::vx::load<Simd>(tmp);
                }

                raze::vx::store[mask, fallback](dst, val);

                for (size_t i = 0; i < N; ++i) {
                    _Type_ expected = mask[i] ? val[i] : fallback[i];
                    raze_assert(dst[i] == expected);
                }
            }

            {
                alignas(64) _Type_ dst[N];
                for (size_t i = 0; i < N; ++i) dst[i] = static_cast<_Type_>(0x87654321);

                Simd val;
                {
                    alignas(64) _Type_ tmp[N];
                    for (size_t j = 0; j < N; ++j) tmp[j] = static_cast<_Type_>(j * 17 + 9);
                    val = raze::vx::load<Simd>[raze::vx::aligned](tmp);
                }

                raze::vx::store[mask, fallback][raze::vx::aligned](dst, val);

                for (size_t i = 0; i < N; ++i) {
                    _Type_ expected = mask[i] ? val[i] : fallback[i];
                    raze_assert(dst[i] == expected);
                }
            }
        };

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 100); ++i) {
            tests_for_mask(make_random_mask<Mask>(), fbk_v);
        }
    }

    void operator()() const {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        //test_size<_Width_ / (sizeof(_Type_) * 8) + 1>();
        //test_size<1>();
    }
};

int main() {
    test_all<variable_length_memory_tests>();
    return 0;
}