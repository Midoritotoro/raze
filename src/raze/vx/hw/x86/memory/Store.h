#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>

__RAZE_VX_NAMESPACE_BEGIN

template <intrin_or_arithmetic_type V>
raze_always_inline void storea_(void* mem, V v) noexcept {
	if constexpr (std::is_same_v<V, __m128i>) return _mm_store_si128(reinterpret_cast<__m128i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128d>) return _mm_store_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128>) return _mm_store_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256i>) return _mm256_store_si256(reinterpret_cast<__m256i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_store_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256>) return _mm256_store_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m512i>) return _mm512_store_si512(mem, v);
	else if constexpr (std::is_same_v<V, __m512d>) return _mm512_store_pd(mem, v);
	else if constexpr (std::is_same_v<V, __m512>) return _mm512_store_ps(mem, v);
	else *static_cast<V*>(mem) = v;
}

template <intrin_or_arithmetic_type V>
raze_always_inline void storeu_(void* mem, V v) noexcept {
	if constexpr (std::is_same_v<V, __m128i>) return _mm_storeu_si128(reinterpret_cast<__m128i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128d>) return _mm_storeu_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128>) return _mm_storeu_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256i>) return _mm256_storeu_si256(reinterpret_cast<__m256i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_storeu_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256>) return _mm256_storeu_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m512i>) return _mm512_storeu_si512(mem, v);
	else if constexpr (std::is_same_v<V, __m512d>) return _mm512_storeu_pd(mem, v);
	else if constexpr (std::is_same_v<V, __m512>) return _mm512_storeu_ps(mem, v);
	else *static_cast<V*>(mem) = v;
}

template <intrin_or_arithmetic_type V, class Policy = unaligned_policy>
raze_always_inline void store_(void* mem, V v, Policy = Policy{}) noexcept {
	if constexpr (is_aligned_v<Policy>) storea_(mem, v);
	else storeu_(mem, v);
}

__RAZE_VX_NAMESPACE_END
