#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Mem>
concept any_iterator_or_pointer = std::input_or_output_iterator<Mem> || std::is_pointer_v<Mem>;

template <arch::ISA ISA, intrin_or_arithmetic_type V>
raze_always_inline V mask_loada_(const void* mem) noexcept {
    if constexpr (std::is_same_v<V, __m128i>) return _mm_load_si128(static_cast<const __m128i*>(mem));
    else if constexpr (std::is_same_v<V, __m128d>) return _mm_load_pd(static_cast<const f64*>(mem));
    else if constexpr (std::is_same_v<V, __m128>)  return _mm_load_ps(static_cast<const f32*>(mem));
    else if constexpr (std::is_same_v<V, __m256i>) return _mm256_load_si256(static_cast<const __m256i*>(mem));
    else if constexpr (std::is_same_v<V, __m256d>) return _mm256_load_pd(static_cast<const f64*>(mem));
    else if constexpr (std::is_same_v<V, __m256>)  return _mm256_load_ps(static_cast<const f32*>(mem));
    else if constexpr (std::is_same_v<V, __m512i>) return _mm512_load_si512(mem);
    else if constexpr (std::is_same_v<V, __m512d>) return _mm512_load_pd(mem);
    else if constexpr (std::is_same_v<V, __m512>)  return _mm512_load_ps(mem);
    else return *static_cast<const V*>(mem);
}

template <arch::ISA ISA, intrin_or_arithmetic_type V>
raze_always_inline V mask_loadu_(const void* mem) noexcept {
    if constexpr (has_sse3<ISA> && sizeof(V) == 16) return as<V>(_mm_lddqu_si128(static_cast<const __m128i*>(mem)));
    else if constexpr (std::is_same_v<V, __m128i>) return _mm_loadu_si128(static_cast<const __m128i*>(mem));
    else if constexpr (std::is_same_v<V, __m128d>) return _mm_loadu_pd(static_cast<const f64*>(mem));
    else if constexpr (std::is_same_v<V, __m128>)  return _mm_loadu_ps(static_cast<const f32*>(mem));
    else if constexpr (has_avx2<ISA> && sizeof(V) == 32) return as<V>(_mm256_lddqu_si256(static_cast<const __m256i*>(mem)));
    else if constexpr (std::is_same_v<V, __m256i>) return _mm256_loadu_si256(static_cast<const __m256i*>(mem));
    else if constexpr (std::is_same_v<V, __m256d>) return _mm256_loadu_pd(static_cast<const f64*>(mem));
    else if constexpr (std::is_same_v<V, __m256>)  return _mm256_loadu_ps(static_cast<const f32*>(mem));
    else if constexpr (std::is_same_v<V, __m512i>) return _mm512_loadu_si512(static_cast<const __m512i*>(mem));
    else if constexpr (std::is_same_v<V, __m512d>) return _mm512_loadu_pd(static_cast<const f64*>(mem));
    else if constexpr (std::is_same_v<V, __m512>)  return _mm512_loadu_ps(static_cast<const f32*>(mem));
    else return *static_cast<const V*>(mem);
}

template <arch::ISA ISA, intrin_or_arithmetic_type V, class Policy = unaligned_policy>
raze_always_inline V operator()(const void* mem, Policy = Policy{}) noexcept {
    if constexpr (is_aligned_v<Policy>) return mask_loada_<ISA, V>(mem);
    else return mask_loadu_<ISA, V>(mem);
}

__RAZE_VX_NAMESPACE_END
