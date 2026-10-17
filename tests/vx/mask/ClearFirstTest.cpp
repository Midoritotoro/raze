#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::clear_first", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;
	using Abi = typename V::abi_type;
    constexpr size_t N = V::size();

	/*if constexpr (Abi::isa == raze::arch::ISA::AVX512BW && sizeof(V) == 64 && sizeof(T) == 1) {
        __debugbreak();
	}*/

    {
        Mask m_empty(false);
        Mask m_empty_cleared = raze::vx::clear_first(m_empty);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!m_empty_cleared[i]);
    }

    {
        Mask m(false);
        m[0] = true;
        auto r = raze::vx::clear_first(m);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!r[i]);
    }

    {
        Mask m(false);
        m[N - 1] = true;
        auto r = raze::vx::clear_first(m);

        for (size_t i = 0; i < N; ++i)
            RTTS_EXPECT(!r[i]);
    }

    if constexpr (Mask::size() > 4) {
        Mask m(false);
        m[1] = true;
        m[3] = true;
        m[N - 1] = true;

        auto r = raze::vx::clear_first(m);

        RTTS_EXPECT(!r[1]);
        RTTS_EXPECT(r[3]);
        RTTS_EXPECT(r[N - 1]);

        for (size_t i = 0; i < N; ++i) {
            if (i != 1 && i != 3 && i != N - 1)
                RTTS_EXPECT(!r[i]);
        }
    }

    {
        Mask m(true);
        auto r = raze::vx::clear_first(m);

        RTTS_EXPECT(!r[0]);

        for (size_t i = 1; i < N; ++i)
            RTTS_EXPECT(r[i]);
    }
};

RAZE_TEST_NAMESPACE_END