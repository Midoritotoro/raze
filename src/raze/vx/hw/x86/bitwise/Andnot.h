#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V bit_andnot_(V x, V y) noexcept {
	using Unsigned = typename IntegerForSizeof<T>::Unsigned;

	if constexpr (std::is_same_v<V, __m128d>) return _mm_andnot_pd(x, y);
	else if constexpr (std::is_same_v<V, __m128i>) return _mm_andnot_si128(x, y);
	else if constexpr (std::is_same_v<V, __m128>) return _mm_andnot_ps(x, y);
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_andnot_pd(x, y);
	else if constexpr (has_avx2<ISA> && std::is_same_v<V, __m256i>) return _mm256_andnot_si256(x, y);
	else if constexpr (sizeof(V) == 32) return as<V>(_mm256_andnot_ps(as<__m256>(x), as<__m256>(y)));
	else if constexpr (has_avx512dq<ISA> && std::is_same_v<V, __m512d>) return _mm512_andnot_pd(x, y);
	else if constexpr (has_avx512dq<ISA> && std::is_same_v<V, __m512>) return _mm512_andnot_ps(x, y);
	else if constexpr (sizeof(V) == 64) return as<V>(_mm512_andnot_si512(as<__m512i>(x), as<__m512i>(y)));
	else return math::bit_cast<V>(Unsigned(Unsigned(~math::bit_cast<Unsigned>(x)) & math::bit_cast<Unsigned>(y)));
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type	V, raw_mask_type M>
raze_always_inline V bit_andnot_(V x, V y, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm_mask_andnot_epi64(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm_mask_andnot_epi32(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm256_mask_andnot_epi64(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm256_mask_andnot_epi32(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
	}
	else if constexpr (sizeof(V) == 64 && has_avx512f<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm512_mask_andnot_epi64(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm512_mask_andnot_epi32(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
	}

	return select_<ISA, T>(bit_andnot_<ISA, T>(x, y), src, mask);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type	M>
raze_always_inline V bit_andnot_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm_maskz_andnot_epi64(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm_maskz_andnot_epi32(mask, as<__m128i>(x), as<__m128i>(y)));
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm256_maskz_andnot_epi64(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm256_maskz_andnot_epi32(mask, as<__m256i>(x), as<__m256i>(y)));
	}
	else if constexpr (sizeof(V) == 64 && has_avx512f<ISA> && std::is_integral_v<M>) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm512_maskz_andnot_epi64(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm512_maskz_andnot_epi32(mask, as<__m512i>(x), as<__m512i>(y)));
	}

	return select_<ISA, T>(bit_andnot_<ISA, T>(x, y), mask);
}

__RAZE_VX_NAMESPACE_END
