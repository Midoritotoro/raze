#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <random>

RTTS_CASE_TPL("raze::vx::is_contiguous", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();

    {
        Mask m(false);
        for (size_t n = 0; n < N; ++n) {
            for (size_t k = n + 1; k < N; ++k) {
                RTTS_EXPECT(!raze::vx::is_contiguous(m, n, k));
            }
        }
    }

    {
        Mask m(true);
        for (size_t n = 0; n < N; ++n) {
            for (size_t k = n; k < N; ++k) {
                RTTS_EXPECT(raze::vx::is_contiguous(m, n, k));
            }
        }
    }

    {
        std::mt19937 rng(123456);
        
        for (int iter = 0; iter < 100; ++iter) {
            Mask m(false);
            
            for (size_t i = 0; i < N; ++i) {
                m[i] = (rng() & 1) != 0;
            }

            for (size_t n = 0; n < N; ++n) {
                for (size_t k = n; k < N; ++k) {
                    bool expected = true;
                    for (size_t i = n; i < k; ++i) {
                        if (!m[i]) {
                            expected = false;
                            break;
                        }
                    }
                    
                    RTTS_EXPECT(raze::vx::is_contiguous(m, n, k) == expected);
                }
            }
        }
    }
};
