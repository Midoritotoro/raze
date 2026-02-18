#pragma once 

#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/type_traits/SimdTypeCheck.h>

#include <raze/compatibility/Inline.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _ToVector_,
    class _FromVector_,
    std::enable_if_t<
        __is_intrin_type_v<std::remove_cvref_t<_FromVector_>> &&
        __is_intrin_type_v<std::remove_cvref_t<_ToVector_>>, int> = 0>
raze_nodiscard raze_always_inline _ToVector_ __intrin_bitcast(_FromVector_ __from) noexcept {
    using _RawFrom_ = std::remove_cvref_t<_FromVector_>;
    using _RawTo_   = std::remove_cvref_t<_ToVector_>;

    if constexpr (std::is_same_v<_RawFrom_, _RawTo_>)
        return __from;

    // Xmm casts 

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castps_si128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castps_pd(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castpd_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castpd_si128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castsi128_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castsi128_pd(__from);

    // Ymm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castps_si256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castps_pd(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castpd_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castpd_si256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castsi256_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castsi256_pd(__from);

    // Xmm to ymm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_zextps128_ps256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castsi256_ps(_mm256_zextsi128_si256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castpd_ps(_mm256_zextpd128_pd256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castps_pd(_mm256_zextps128_ps256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castsi256_pd(_mm256_zextsi128_si256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_zextpd128_pd256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castps_si256(_mm256_zextps128_ps256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_zextsi128_si256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castpd_si256(_mm256_zextpd128_pd256(__from));

    // Ymm to xmm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m128>)
        return _mm256_castps256_ps128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castsi128_ps(_mm256_castsi256_si128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castpd_ps(_mm256_castpd256_pd128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castps_pd(_mm256_castps256_ps128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castsi128_pd(_mm256_castsi256_si128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m128d>)
        return _mm256_castpd256_pd128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castps_si128(_mm256_castps256_ps128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m128i>)
        return _mm256_castsi256_si128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castpd_si128(_mm256_castpd256_pd128(__from));

    // Zmm casts 

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castps_si512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castps_pd(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castpd_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castpd_si512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castsi512_ps(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castsi512_pd(__from);

    // Zmm to ymm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castps_si256(_mm512_castps512_ps256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m256>)
        return _mm512_castps512_ps256(__from);
    
    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castps_pd(_mm512_castps512_ps256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m256i>)
        return _mm512_castsi512_si256(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castsi256_ps(_mm512_castsi512_si256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m256d>)
        return _mm256_castsi256_pd(_mm512_castsi512_si256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m256i>)
        return _mm256_castpd_si256(_mm512_castpd512_pd256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m256>)
        return _mm256_castpd_ps(_mm512_castpd512_pd256(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m256d>)
        return _mm512_castpd512_pd256(__from);

    // Zmm to xmm casts 

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castps_si128(_mm512_castps512_ps128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m128>)
        return _mm512_castps512_ps128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castps_pd(_mm512_castps512_ps128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m128i>)
        return _mm512_castsi512_si128(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castsi128_ps(_mm512_castsi512_si128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512i> && std::is_same_v<_RawTo_, __m128d>)
        return _mm_castsi128_pd(_mm512_castsi512_si128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m128i>)
        return _mm_castpd_si128(_mm512_castpd512_pd128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m128>)
        return _mm_castpd_ps(_mm512_castpd512_pd128(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m512d> && std::is_same_v<_RawTo_, __m128d>)
        return _mm512_castpd512_pd128(__from);

    // Ymm to zmm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castps_si512(_mm512_zextps256_ps512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castpd_si512(_mm512_zextpd256_pd512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_zextsi256_si512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castps_pd(_mm512_zextps256_ps512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_zextpd256_pd512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castsi512_pd(_mm512_zextsi256_si512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_zextps256_ps512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m256d> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castpd_ps(_mm512_zextpd256_pd512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m256i> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castsi512_ps(_mm512_zextsi256_si512(__from));

    // Xmm to zmm casts

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castps_si512(_mm512_zextps128_ps512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_castpd_si512(_mm512_zextpd128_pd512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m512i>)
        return _mm512_zextsi128_si512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castps_pd(_mm512_zextps128_ps512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_zextpd128_pd512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m512d>)
        return _mm512_castsi512_pd(_mm512_zextsi128_si512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_zextps128_ps512(__from);

    else if constexpr (std::is_same_v<_RawFrom_, __m128d> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castpd_ps(_mm512_zextpd128_pd512(__from));

    else if constexpr (std::is_same_v<_RawFrom_, __m128i> && std::is_same_v<_RawTo_, __m512>)
        return _mm512_castsi512_ps(_mm512_zextsi128_si512(__from));
}


__RAZE_DATAPAR_NAMESPACE_END
