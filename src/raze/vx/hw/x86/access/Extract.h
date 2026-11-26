#pragma once 

#include <raze/math/BitCast.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype I>
consteval raze_always_inline auto broadcast_pshufd_index_(std::integral_constant<sizetype, I> i) noexcept {
	return (i & 0x03) | (i << 2) | (i << 4) | (i << 6);
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_always_inline T extract_first_(V x) noexcept {
	if constexpr (epi64<T> || epu64<T>) {
#if defined(raze_processor_x86_32)
		const auto v = as<__m128i>(x);
		const auto lo = static_cast<u64>(static_cast<u32>(_mm_cvtsi128_si32(v)));
		const auto hi = static_cast<u64>(static_cast<u32>(_mm_cvtsi128_si32(_mm_srli_si128(v, 4))));
		return static_cast<T>(lo | (hi << 32));
#else
		return static_cast<T>(_mm_cvtsi128_si64(as<__m128i>(x)));
#endif
	}
	else if constexpr (epi32<T> || epu32<T>) return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(x)));
	else if constexpr (epi16<T> || epu16<T>) return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(x)) & 0xFFFF);
	else if constexpr (epi8<T> || epu8<T>) return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(x)) & 0xFF);
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
		store_(array, x, aligned_policy{});

		return array[i & (length - 1)];
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, sizetype I>
raze_always_inline T extract_(V v, std::integral_constant<sizetype, I> i) noexcept {
	if constexpr (arithmetic_type<V>) return v;
	else if constexpr (i == 0) return extract_first_<ISA, T>(v);
	else if constexpr (sizeof(V) == 16) {
#if !defined(raze_processor_x86_32)
		if constexpr (sizeof(T) == 8 && std::is_integral_v<T> && has_sse41<ISA>) return static_cast<T>(_mm_extract_epi64(as<__m128i>(v), I));
		else
#endif
		if constexpr (sizeof(T) == 4) {
			if constexpr(has_sse41<ISA> && !std::is_floating_point_v<T>) return static_cast<T>(_mm_extract_epi32(as<__m128i>(v), I));
			else if constexpr (std::is_floating_point_v<T>) return _mm_cvtss_f32(as<__m128>(_mm_shuffle_epi32(as<__m128i>(v), broadcast_pshufd_index_(i))));
			else if constexpr (std::is_integral_v<T>) return static_cast<T>(_mm_cvtsi128_si32(_mm_shuffle_epi32(as<__m128i>(v), broadcast_pshufd_index_(i))));
		}
		else if constexpr (sizeof(T) == 2) return static_cast<T>(_mm_extract_epi16(as<__m128i>(v), I));
		else if constexpr (sizeof(T) == 1 && has_sse41<ISA>) return static_cast<T>(_mm_extract_epi8(as<__m128i>(v), I));
	}
	else if constexpr (sizeof(V) == 32 && has_avx2<ISA>) {
#if !defined(raze_processor_x86_32)
		if constexpr (sizeof(T) == 8 && std::is_integral_v<T>) return static_cast<T>(_mm256_extract_epi64(as<__m256i>(v), I));
		else
#endif
		if constexpr (sizeof(T) == 4 && std::is_integral_v<T>) return static_cast<T>(_mm256_extract_epi32(as<__m256i>(v), I));
		else if constexpr (sizeof(T) == 2) return static_cast<T>(_mm256_extract_epi16(as<__m256i>(v), I));
		else if constexpr (sizeof(T) == 1) return static_cast<T>(_mm256_extract_epi8(as<__m256i>(v), I));
	}
	else if constexpr (sizeof(V) == 64) {
		constexpr auto mask = 1ull << I;
		if constexpr (pd<T>) return _mm512_cvtsd_f64(_mm512_maskz_compress_pd(mask, as<__m512d>(v)));
		else if constexpr (ps<T>) return _mm512_cvtss_f32(_mm512_maskz_compress_ps(mask, as<__m512>(v)));
		else if constexpr (epi32<T> || epu32<T>) return static_cast<T>(_mm512_cvtsi512_si32(_mm512_maskz_compress_epi32(mask, as<__m512i>(v))));
#if !defined(raze_processor_x86_32)
		else if constexpr (epi64<T> || epu64<T>) return static_cast<T>(_mm_cvtsi128_si64(as<__m128i>(_mm512_maskz_compress_epi64(mask, as<__m512i>(v)))));
#endif
	}

	return extract_<ISA, T>(v, static_cast<u8>(i.value));
}

template <intrin_type Extract, arch::ISA ISA, intrin_type V, sizetype I>
raze_always_inline Extract extract_vector_(V v, std::integral_constant<sizetype, I>) noexcept {
	if constexpr (sizeof(V) == sizeof(Extract)) return as<Extract>(v);
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(Extract) == 16) {
			if constexpr (has_avx2<ISA>) return as<Extract>(_mm256_extracti128_si256(as<__m256i>(v), I));
			else return as<Extract>(_mm256_extractf128_si256(as<__m256i>(v), I));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(Extract) == 16) return as<Extract>(_mm512_extracti32x4_epi32(as<__m512i>(v), I));
		else if constexpr (sizeof(Extract) == 32) return as<Extract>(_mm512_extracti64x4_epi64(as<__m512i>(v), I));
	}
}

__RAZE_VX_NAMESPACE_END
