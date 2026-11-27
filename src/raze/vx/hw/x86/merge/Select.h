#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>

__RAZE_VX_NAMESPACE_BEGIN 

template <intrin_or_arithmetic_type V>
raze_always_inline V select_zero_() noexcept {
	if constexpr (std::is_same_v<V, __m128i>) return _mm_setzero_si128();
	else if constexpr (std::is_same_v<V, __m128d>) return _mm_setzero_pd();
	else if constexpr (std::is_same_v<V, __m128>) return _mm_setzero_ps();
	else if constexpr (std::is_same_v<V, __m256i>) return _mm256_setzero_si256();
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_setzero_pd();
	else if constexpr (std::is_same_v<V, __m256>) return _mm256_setzero_ps();
	else if constexpr (std::is_same_v<V, __m512i>) return _mm512_setzero_si512();
	else if constexpr (std::is_same_v<V, __m512d>) return _mm512_setzero_pd();
	else if constexpr (std::is_same_v<V, __m512>) return _mm512_setzero_ps();
	else return V(0);
}

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
	else {
		const auto mv = to_vector_<ISA, V, T>(mask);
		if constexpr (sizeof(V) == 16)
			return as<V>(_mm_or_si128(_mm_and_si128(as<__m128i>(mv), as<__m128i>(x)),
				_mm_andnot_si128(as<__m128i>(mv), as<__m128i>(y))));
		else if constexpr (sizeof(V) == 32) {
			if constexpr (has_avx2<ISA>)
				return as<V>(_mm256_or_si256(_mm256_and_si256(as<__m256i>(mv), as<__m256i>(x)),
					_mm256_andnot_si256(as<__m256i>(mv), as<__m256i>(y))));
			else
				return as<V>(_mm256_or_ps(_mm256_and_ps(as<__m256>(mv), as<__m256>(x)),
					_mm256_andnot_ps(as<__m256>(mv), as<__m256>(y))));
		}
		else
			return as<V>(_mm512_ternarylogic_epi32(as<__m512i>(mv), as<__m512i>(x), as<__m512i>(y), 0xCA));
	}
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

	return select_<ISA, T>(x, select_zero_<V>(), mask);
}


__RAZE_VX_NAMESPACE_END