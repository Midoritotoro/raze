#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V select_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (std::is_integral_v<M> && has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_mask_blend_epi64(mask, as<__m128i>(y), as<__m128i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_mask_blend_epi32(mask, as<__m128i>(y), as<__m128i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm_mask_blend_ps(mask, as<__m128>(y), as<__m128>(x)));
			else if constexpr (pd<T>) return as<V>(_mm_mask_blend_pd(mask, as<__m128d>(y), as<__m128d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_mask_blend_epi16(mask, as<__m128i>(y), as<__m128i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_mask_blend_epi8(mask, as<__m128i>(y), as<__m128i>(x)));
			}
		}
		else if constexpr (intrin_type<M>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_ternarylogic_epi32(as<__m128i>(mask), as<__m128i>(x), as<__m128i>(y), 0xCA));
			else if constexpr (has_sse41<ISA>) {
				if constexpr (sizeof(T) == 8) return as<V>(_mm_blendv_pd(as<__m128d>(y), as<__m128d>(x), as<__m128d>(mask)));
				else if constexpr (sizeof(T) == 4) return as<V>(_mm_blendv_ps(as<__m128>(y), as<__m128>(x), as<__m128>(mask)));
				else return as<V>(_mm_blendv_epi8(as<__m128i>(y), as<__m128i>(x), as<__m128i>(mask)));
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (std::is_integral_v<M> && has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_mask_blend_epi64(mask, as<__m256i>(y), as<__m256i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mask_blend_epi32(mask, as<__m256i>(y), as<__m256i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm256_mask_blend_ps(mask, as<__m256>(y), as<__m256>(x)));
			else if constexpr (pd<T>) return as<V>(_mm256_mask_blend_pd(mask, as<__m256d>(y), as<__m256d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_mask_blend_epi16(mask, as<__m256i>(y), as<__m256i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_mask_blend_epi8(mask, as<__m256i>(y), as<__m256i>(x)));
			}
		}
		else if constexpr (intrin_type<M>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_ternarylogic_epi32(as<__m256i>(mask), as<__m256i>(x), as<__m256i>(y), 0xCA));
			else if constexpr (has_avx2<ISA>) {
				if constexpr (sizeof(T) == 8) return as<V>(_mm256_blendv_pd(as<__m256d>(y), as<__m256d>(x), as<__m256d>(mask)));
				else if constexpr (sizeof(T) == 4) return as<V>(_mm256_blendv_ps(as<__m256>(y), as<__m256>(x), as<__m256>(mask)));
				else return as<V>(_mm256_blendv_epi8(as<__m256i>(y), as<__m256i>(x), as<__m256i>(mask)));
			}
			else if constexpr (has_avx<ISA>) {
				if constexpr (sizeof(T) == 8) return as<V>(_mm256_blendv_pd(as<__m256d>(y), as<__m256d>(x), as<__m256d>(mask)));
				else if constexpr (sizeof(T) == 4) return as<V>(_mm256_blendv_ps(as<__m256>(y), as<__m256>(x), as<__m256>(mask)));
			}
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (std::is_integral_v<M>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_blend_epi64(mask, as<__m512i>(y), as<__m512i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_blend_epi32(mask, as<__m512i>(y), as<__m512i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm512_mask_blend_ps(mask, as<__m512>(y), as<__m512>(x)));
			else if constexpr (pd<T>) return as<V>(_mm512_mask_blend_pd(mask, as<__m512d>(y), as<__m512d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_mask_blend_epi16(mask, as<__m512i>(y), as<__m512i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_mask_blend_epi8(mask, as<__m512i>(y), as<__m512i>(x)));
			}
		}
		else if constexpr (intrin_type<M>) {
			return as<V>(_mm512_ternarylogic_epi32(as<__m512i>(mask), as<__m512i>(x), as<__m512i>(y), 0xCA));
		}
	}

	if constexpr (arithmetic_type<V>) return mask ? x : y;
	else return _Ternarylogic<ISA, T>()(_To_vector<ISA, V, T>()(mask), x, y, std::integral_constant<u8, 0xca>{});
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V select_(V x, M	mask) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (std::is_integral_v<M> && has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_maskz_mov_epi64(mask, as<__m128i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_maskz_mov_epi32(mask, as<__m128i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm_maskz_mov_ps(mask, as<__m128>(x)));
			else if constexpr (pd<T>) return as<V>(_mm_maskz_mov_pd(mask, as<__m128d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_maskz_mov_epi16(mask, as<__m128i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_maskz_mov_epi8(mask, as<__m128i>(x)));
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (std::is_integral_v<M> && has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_maskz_mov_epi64(mask, as<__m256i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_maskz_mov_epi32(mask, as<__m256i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm256_maskz_mov_ps(mask, as<__m256>(x)));
			else if constexpr (pd<T>) return as<V>(_mm256_maskz_mov_pd(mask, as<__m256d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_maskz_mov_epi16(mask, as<__m256i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_maskz_mov_epi8(mask, as<__m256i>(x)));
			}
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (std::is_integral_v<M> && has_avx512f<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_mov_epi64(mask, as<__m512i>(x)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_mov_epi32(mask, as<__m512i>(x)));
			else if constexpr (ps<T>) return as<V>(_mm512_maskz_mov_ps(mask, as<__m512>(x)));
			else if constexpr (pd<T>) return as<V>(_mm512_maskz_mov_pd(mask, as<__m512d>(x)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_maskz_mov_epi16(mask, as<__m512i>(x)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_maskz_mov_epi8(mask, as<__m512i>(x)));
			}
		}
	}

	return select_<ISA, T>(x, _Zero<ISA, V>()(), mask);
}


__RAZE_VX_NAMESPACE_END