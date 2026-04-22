#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA                   _ISA_,
    intrin_or_arithmetic_type   _Type_>
struct _Load {
    static raze_always_inline _Type_ __load_aligned(const void* __memory) noexcept {
        if constexpr (std::is_same_v<_Type_, __m128i>) return _mm_load_si128(static_cast<const __m128i*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m128d>) return _mm_load_pd(static_cast<const double*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m128>)  return _mm_load_ps(static_cast<const float*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m256i>) return _mm256_load_si256(static_cast<const __m256i*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m256d>) return _mm256_load_pd(static_cast<const double*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m256>)  return _mm256_load_ps(static_cast<const float*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m512i>) return _mm512_load_si512(__memory);
        else if constexpr (std::is_same_v<_Type_, __m512d>) return _mm512_load_pd(__memory);
        else if constexpr (std::is_same_v<_Type_, __m512>)  return _mm512_load_ps(__memory);
        else return *static_cast<const _Type_*>(__memory);
    }

    static raze_always_inline _Type_ __load_unaligned(const void* __memory) noexcept {
        if constexpr (__has_sse3_support_v<_ISA_> && sizeof(_Type_) == 16) return __as<_Type_>(_mm_lddqu_si128(static_cast<const __m128i*>(__memory)));
        else if constexpr (std::is_same_v<_Type_, __m128i>) return _mm_loadu_si128(static_cast<const __m128i*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m128d>) return _mm_loadu_pd(static_cast<const double*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m128>)  return _mm_loadu_ps(static_cast<const float*>(__memory));
        else if constexpr (__has_avx2_support_v<_ISA_> && sizeof(_Type_) == 32) return __as<_Type_>(_mm256_lddqu_si256(static_cast<const __m256i*>(__memory)));
        else if constexpr (std::is_same_v<_Type_, __m256i>) return _mm256_loadu_si256(static_cast<const __m256i*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m256d>) return _mm256_loadu_pd(static_cast<const double*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m256>)  return _mm256_loadu_ps(static_cast<const float*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m512i>) return _mm512_loadu_si512(static_cast<const __m512i*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m512d>) return _mm512_loadu_pd(static_cast<const double*>(__memory));
        else if constexpr (std::is_same_v<_Type_, __m512>)  return _mm512_loadu_ps(static_cast<const float*>(__memory));
        else return *static_cast<const _Type_*>(__memory);
    }

    template <class _AlignmentPolicy_ = __unaligned_policy>
    raze_nodiscard raze_static_operator raze_always_inline _Type_ operator()(
        const void* __memory, 
        _AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept 
    {
        if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment)
            return __load_aligned(__memory);
        else
            return __load_unaligned(__memory);
    }
};

__RAZE_VX_NAMESPACE_END
