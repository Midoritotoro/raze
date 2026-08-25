#pragma once 

#include <raze/math/BitCast.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype I>
consteval auto broadcast_pshufd_index_(std::integral_constant<sizetype, I> i) noexcept {
	return (i & 0x03) | (i << 2) | (i << 4) | (i << 6);
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_always_inline T extract_first_(V x) noexcept {
	if constexpr (epi64<T> || epu64<T>) return _mm_cvtsi128_si64x(as<__m128i>(x));
	else if constexpr (epi32<T> || epu32<T>) return _mm_cvtsi128_si32(as<__m128i>(x));
	else if constexpr (epi16<T> || epu16<T>) return _mm_cvtsi128_si32(as<__m128i>(x)) & 0xFFFF;
	else if constexpr (epi8<T> || epu8<T>) return _mm_cvtsi128_si32(as<__m128i>(x)) & 0xFF;
	else if constexpr (pd<T>) return _mm_cvtsd_f64(as<__m128d>(x));
	else if constexpr (ps<T>) return _mm_cvtss_f32(as<__m128>(x));
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline T extract_(V x, u8 i) noexcept {
	if constexpr (arithmetic_type<V>) {
		return x;
	}
	else {
		constexpr auto length = sizeof(V) / sizeof(T);

		alignas(sizeof(V)) T array[length];
		_Store<ISA>()(array, x);

		return array[i & (length - 1)];
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, sizetype I>
raze_always_inline T extract_(V v, std::integral_constant<sizetype, I> i) noexcept {
	if constexpr (arithmetic_type<V>) return v;
	else if constexpr (i == 0) return extract_first_<_ISA_, T>(v);
	else if constexpr (sizeof(V) == 16) {
		if constexpr (sizeof(T) == 8 && std::is_integral_v<T> && has_sse41<_ISA_>) return _mm_extract_epi64(as<__m128i>(v), i);
		else if constexpr (sizeof(T) == 4) {
			if constexpr(has_sse41<_ISA_> && !std::is_floating_point_v<T>) return _mm_extract_epi32(as<__m128i>(v), i);
			else if constexpr (std::is_floating_point_v<T>) return _mm_cvtss_f32(as<__m128>(_mm_shuffle_epi32(as<__m128i>(v), broadcast_pshufd_index_(i))));
			else if constexpr (std::is_floating_point_v<T>) return _mm_cvtsi128_si32(_mm_shuffle_epi32(as<__m128i>(v), broadcast_pshufd_index_(i)));
		}
		else if constexpr (sizeof(T) == 2) return _mm_extract_epi16(as<__m128i>(v), i);
		else if constexpr (sizeof(T) == 1) return _mm_extract_epi8(as<__m128i>(v), i);
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(T) == 8 && std::is_integral_v<T>) return _mm256_extract_epi64(as<__m256i>(v), i);
		else if constexpr (sizeof(T) == 4 && std::is_integral_v<T>) return _mm256_extract_epi32(as<__m256i>(v), i);
		else if constexpr (sizeof(T) == 2 && has_avx2<_ISA_>) return _mm256_extract_epi16(as<__m256i>(v), i);
		else if constexpr (sizeof(T) == 1 && has_avx2<_ISA_>) return _mm256_extract_epi8(as<__m256i>(v), i);
	}
	else if constexpr (sizeof(V) == 64) {
		constexpr auto mask = 1u << I;
		if constexpr (pd<T>) return _mm512_cvtsd_f64(_mm512_maskz_compress_pd(mask, as<__m512d>(v)));
		else if constexpr (ps<T>) return _mm512_cvtss_f32(_mm512_maskz_compress_ps(mask, as<__m512>(v)));
		else if constexpr (epi32<T> || epu32<T>) return _mm512_cvtsi512_si32(_mm512_maskz_compress_epi32(mask, as<__m512i>(v)));
		else if constexpr (epi64<T> || epu64<T>) return _mm_cvtsi128_si64(as<__m128i>(_mm512_maskz_compress_epi64(mask, as<__m512i>(v))));
	}

	return extract_<ISA, T>(v, i.value);
}

template <intrin_type Extract, arch::ISA _ISA_, intrin_type V, sizetype I>
raze_always_inline Extract extract_vector_(V v, std::integral_constant<sizetype, I> i) noexcept {
	if constexpr (sizeof(V) == sizeof(Extract)) return as<Extract>(v);
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(Extract) == 16) return as<Extract>(_mm256_extracti128_si256(as<__m256i>(v), i));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(Extract) == 16) return as<Extract>(_mm512_extracti64x2_epi64(as<__m512i>(v), i));
		else if constexpr (sizeof(Extract) == 32) return as<Extract>(_mm512_extracti64x4_epi64(as<__m512i>(v), i));
	}
}

__RAZE_VX_NAMESPACE_END
