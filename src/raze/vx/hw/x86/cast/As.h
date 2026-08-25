#pragma once 

#include <src/raze/vx/hw/x86/cast/SimdIntegralTypesCheck.h>
#include <src/raze/traits/SimdTypeCheck.h>
#include <raze/compatibility/Inline.h>
#include <raze/math/BitCast.h>


__RAZE_VX_NAMESPACE_BEGIN

template <intrin_type ToVector, intrin_type FromVector>
raze_nodiscard raze_always_inline ToVector as(FromVector from) noexcept {
    using From = std::remove_cvref_t<FromVector>;
    using To = std::remove_cvref_t<ToVector>;

    if constexpr (std::is_same_v<From, To>)                                           return from;
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m128i>)   return _mm_castps_si128(from);
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m128d>)   return _mm_castps_pd(from);
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m128>)   return _mm_castpd_ps(from);
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m128i>)  return _mm_castpd_si128(from);
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m128>)   return _mm_castsi128_ps(from);
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m128d>)  return _mm_castsi128_pd(from);
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m256i>)   return _mm256_castps_si256(from);
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m256d>)   return _mm256_castps_pd(from);
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m256>)   return _mm256_castpd_ps(from);
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m256i>)  return _mm256_castpd_si256(from);
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m256>)   return _mm256_castsi256_ps(from);
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m256d>)  return _mm256_castsi256_pd(from);
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m256>)    return _mm256_castps128_ps256(from);
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m256>)   return _mm256_castsi256_ps(_mm256_castsi128_si256(from));
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m256>)   return _mm256_castpd_ps(_mm256_castpd128_pd256(from));
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m256d>)   return _mm256_castps_pd(_mm256_castps128_ps256(from));
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m256d>)  return _mm256_castsi256_pd(_mm256_castsi128_si256(from));
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m256d>)  return _mm256_castpd128_pd256(from);
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m256i>)   return _mm256_castps_si256(_mm256_castps128_ps256(from));
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m256i>)  return _mm256_castsi128_si256(from);
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m256i>)  return _mm256_castpd_si256(_mm256_castpd128_pd256(from));
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m128>)    return _mm256_castps256_ps128(from);
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m128>)   return _mm_castsi128_ps(_mm256_castsi256_si128(from));
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m128>)   return _mm_castpd_ps(_mm256_castpd256_pd128(from));
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m128d>)   return _mm_castps_pd(_mm256_castps256_ps128(from));
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m128d>)  return _mm_castsi128_pd(_mm256_castsi256_si128(from));
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m128d>)  return _mm256_castpd256_pd128(from);
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m128i>)   return _mm_castps_si128(_mm256_castps256_ps128(from));
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m128i>)  return _mm256_castsi256_si128(from);
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m128i>)  return _mm_castpd_si128(_mm256_castpd256_pd128(from));
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m512i>)   return _mm512_castps_si512(from);
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m512d>)   return _mm512_castps_pd(from);
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m512>)   return _mm512_castpd_ps(from);
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m512i>)  return _mm512_castpd_si512(from);
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m512>)   return _mm512_castsi512_ps(from);
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m512d>)  return _mm512_castsi512_pd(from);
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m256i>)   return _mm256_castps_si256(_mm512_castps512_ps256(from));
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m256>)    return _mm512_castps512_ps256(from);
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m256d>)   return _mm256_castps_pd(_mm512_castps512_ps256(from));
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m256i>)  return _mm512_castsi512_si256(from);
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m256>)   return _mm256_castsi256_ps(_mm512_castsi512_si256(from));
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m256d>)  return _mm256_castsi256_pd(_mm512_castsi512_si256(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m256i>)  return _mm256_castpd_si256(_mm512_castpd512_pd256(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m256>)   return _mm256_castpd_ps(_mm512_castpd512_pd256(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m256d>)  return _mm512_castpd512_pd256(from);
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m128i>)   return _mm_castps_si128(_mm512_castps512_ps128(from));
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m128>)    return _mm512_castps512_ps128(from);
    else if constexpr (std::is_same_v<From, __m512> && std::is_same_v<To, __m128d>)   return _mm_castps_pd(_mm512_castps512_ps128(from));
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m128i>)  return _mm512_castsi512_si128(from);
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m128>)   return _mm_castsi128_ps(_mm512_castsi512_si128(from));
    else if constexpr (std::is_same_v<From, __m512i> && std::is_same_v<To, __m128d>)  return _mm_castsi128_pd(_mm512_castsi512_si128(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m128i>)  return _mm_castpd_si128(_mm512_castpd512_pd128(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m128>)   return _mm_castpd_ps(_mm512_castpd512_pd128(from));
    else if constexpr (std::is_same_v<From, __m512d> && std::is_same_v<To, __m128d>)  return _mm512_castpd512_pd128(from);
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m512i>)   return _mm512_castps_si512(_mm512_castps256_ps512(from));
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m512i>)  return _mm512_castpd_si512(_mm512_castpd256_pd512(from));
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m512i>)  return _mm512_castsi256_si512(from);
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m512d>)   return _mm512_castps_pd(_mm512_castps256_ps512(from));
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m512d>)  return _mm512_castpd256_pd512(from);
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m512d>)  return _mm512_castsi512_pd(_mm512_castsi256_si512(from));
    else if constexpr (std::is_same_v<From, __m256> && std::is_same_v<To, __m512>)    return _mm512_castps256_ps512(from);
    else if constexpr (std::is_same_v<From, __m256d> && std::is_same_v<To, __m512>)   return _mm512_castpd_ps(_mm512_castpd256_pd512(from));
    else if constexpr (std::is_same_v<From, __m256i> && std::is_same_v<To, __m512>)   return _mm512_castsi512_ps(_mm512_castsi256_si512(from));
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m512i>)   return _mm512_castps_si512(_mm512_castps128_ps512(from));
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m512i>)  return _mm512_castpd_si512(_mm512_castpd128_pd512(from));
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m512i>)  return _mm512_castsi128_si512(from);
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m512d>)   return _mm512_castps_pd(_mm512_castps128_ps512(from));
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m512d>)  return _mm512_castpd128_pd512(from);
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m512d>)  return _mm512_castsi512_pd(_mm512_castsi128_si512(from));
    else if constexpr (std::is_same_v<From, __m128> && std::is_same_v<To, __m512>)    return _mm512_castps128_ps512(from);
    else if constexpr (std::is_same_v<From, __m128d> && std::is_same_v<To, __m512>)   return _mm512_castpd_ps(_mm512_castpd128_pd512(from));
    else if constexpr (std::is_same_v<From, __m128i> && std::is_same_v<To, __m512>)   return _mm512_castsi512_ps(_mm512_castsi128_si512(from));
}

__RAZE_VX_NAMESPACE_END
