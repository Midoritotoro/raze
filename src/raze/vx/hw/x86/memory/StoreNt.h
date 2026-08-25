#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V>
raze_no_stack_protector raze_always_inline void store_nt_(void* raze_restrict mem, V v) noexcept {
	if constexpr (std::is_same_v<V, __m128i>) return _mm_stream_si128(reinterpret_cast<__m128i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128d>) return _mm_stream_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m128>) return _mm_stream_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256i>) return _mm256_stream_si256(reinterpret_cast<__m256i*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_stream_pd(reinterpret_cast<f64*>(mem), v);
	else if constexpr (std::is_same_v<V, __m256>) return _mm256_stream_ps(reinterpret_cast<f32*>(mem), v);
	else if constexpr (std::is_same_v<V, __m512i>) return _mm512_stream_si512(mem, v);
	else if constexpr (std::is_same_v<V, __m512d>) return _mm512_stream_pd(mem, v);
	else if constexpr (std::is_same_v<V, __m512>) return _mm512_stream_ps(mem, v);
	else *static_cast<V*>(mem) = v;
}

__RAZE_VX_NAMESPACE_END
