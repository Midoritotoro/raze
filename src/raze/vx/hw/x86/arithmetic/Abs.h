#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V abs_(V x) noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return x;

	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm_abs_epi64(x);
			else {
				const auto high_sign = _mm_srai_epi32(x, 31);
				const auto sign = _mm_shuffle_epi32(high_sign, 0xF5);

				const auto invert = _mm_xor_si128(x, sign);
				return _mm_sub_epi64(invert, sign);
			}
		}
		else if constexpr (epi32<T>) {
			if constexpr (has_ssse3<ISA>) return _mm_abs_epi32(x);
			else {
				const auto sign = _mm_srai_epi32(as<__m128i>(x), 31);
				const auto invert = _mm_xor_si128(as<__m128i>(x), sign);

				return as<V>(_mm_sub_epi32(invert, sign));
			}
		}
		else if constexpr (epi16<T>) {
			if constexpr (has_ssse3<ISA>) return as<V>(_mm_abs_epi16(as<__m128i>(x)));
			else {
				const auto negate = _mm_sub_epi16(_mm_setzero_si128(), as<__m128i>(x));
				return _mm_max_epi16(as<__m128i>(x), negate);
			}
		}
		else if constexpr (epi8<T>) {
			if constexpr (has_ssse3<ISA>) return as<V>(_mm_abs_epi8(as<__m128i>(x)));
			else {
				const auto negate = _mm_sub_epi8(_mm_setzero_si128(), as<__m128i>(x));
				return as<V>(_mm_min_epu8(as<__m128i>(x), negate));
			}
		}
		else if constexpr (ps<T>) return as<V>(_mm_and_ps(as<__m128>(x), as<__m128>(_mm_set1_epi32(0x7FFFFFFF))));
		else if constexpr (pd<T>) return as<V>(_mm_and_pd(as<__m128d>(x), as<__m128d>(_mm_setr_epi32(-1, 0x7FFFFFFFu, -1, 0x7FFFFFFFu))));
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (pd<T>) return as<V>(_mm256_and_pd(as<__m256d>(x), as<__m256d>(_mm256_setr_epi32(
			0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu))));
		else if constexpr (ps<T>) return as<V>(_mm256_and_ps(as<__m256>(x), as<__m256>(_mm256_set1_epi32(0x7FFFFFFFu))));
		else if constexpr (has_avx2<ISA>) {
			if constexpr (epi64<T>) {
				if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_abs_epi64(as<__m256i>(x)));
				else {
					const auto sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), x);
					const auto inverted = _mm256_xor_si256(as<__m256i>(x), sign);

					return as<V>(_mm256_sub_epi64(inverted, sign));
				}
			}
			else if constexpr (epi32<T>) return as<V>(_mm256_abs_epi32(as<__m256i>(x)));
			else if constexpr (epi16<T>) return as<V>(_mm256_abs_epi16(as<__m256i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm256_abs_epi8(as<__m256i>(x)));
		}
		else {
			const auto low = abs_<arch::ISA::SSE42, T>(as<__m128i>(x));
			const auto high = abs_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1));

			return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_abs_epi64(as<__m512i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm512_abs_epi32(as<__m512i>(x)));
		else if constexpr (ps<T>) return as<V>(_mm512_abs_ps(as<__m512>(x)));
		else if constexpr (pd<T>) return as<V>(_mm512_abs_pd(as<__m512d>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_abs_epi16(as<__m512i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm512_abs_epi8(as<__m512i>(x)));
		}
		else {
			const auto low = as<__m256i>(x);
			const auto high = _mm512_extractf64x4_pd(as<__m512d>(x), 1);

			auto r = as<__m512i>(abs_<arch::ISA::AVX2, T>(low));
			r = as<__m512i>(_mm512_insertf64x4(as<__m512d>(r),
				as<__m256d>(abs_<arch::ISA::AVX2, T>(high)), 1));

			return as<V>(r);
		}
	}
	else return x < 0 ? -x : x;
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V abs_(V x, M mask) noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return select_<ISA, T>(x, mask);
	
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_maskz_abs_epi64(mask, as<__m128i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm_maskz_abs_epi32(mask, as<__m128i>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_maskz_abs_epi16(mask, as<__m128i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm_maskz_abs_epi8(mask, as<__m128i>(x)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_maskz_abs_epi64(mask, as<__m256i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm256_maskz_abs_epi32(mask, as<__m256i>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_maskz_abs_epi16(mask, as<__m256i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm256_maskz_abs_epi8(mask, as<__m256i>(x)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_maskz_abs_epi64(mask, as<__m512i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm512_maskz_abs_epi32(mask, as<__m512i>(x)));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_abs_ps(mask, as<__m512>(x)));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_abs_pd(mask, as<__m512d>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_maskz_abs_epi16(mask, as<__m512i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm512_maskz_abs_epi8(mask, as<__m512i>(x)));
		}
	}
	
	if constexpr (arithmetic_type<V>) return (mask) ? (x < 0 ? -x : x) : 0;
	else return select_<ISA, T>(abs_<ISA, T>(x), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V abs_(V x, M mask, V src) noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return select_<ISA, T>(x, src, mask);

	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_mask_abs_epi64(as<__m128i>(src), mask, as<__m128i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm_mask_abs_epi32(as<__m128i>(src), mask, as<__m128i>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_mask_abs_epi16(as<__m128i>(src), mask, as<__m128i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm_mask_abs_epi8(as<__m128i>(src), mask, as<__m128i>(x)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_mask_abs_epi64(as<__m256i>(src), mask, as<__m256i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm256_mask_abs_epi32(as<__m256i>(src), mask, as<__m256i>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_mask_abs_epi16(as<__m256i>(src), mask, as<__m256i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm256_mask_abs_epi8(as<__m256i>(src), mask, as<__m256i>(x)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_mask_abs_epi64(as<__m512i>(src), mask, as<__m512i>(x)));
		else if constexpr (epi32<T>) return as<V>(_mm512_mask_abs_epi32(as<__m512i>(src), mask, as<__m512i>(x)));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_abs_ps(as<__m512>(src), mask, as<__m512>(x)));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_abs_pd(as<__m512d>(src), mask, as<__m512d>(x)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_mask_abs_epi16(as<__m512i>(src), mask, as<__m512i>(x)));
			else if constexpr (epi8<T>) return as<V>(_mm512_mask_abs_epi8(as<__m512i>(src), mask, as<__m512i>(x)));
		}
	}
	
	if constexpr (arithmetic_type<V>) return (mask) ? (x < 0 ? -x : x) : src;
	else return select_<ISA, T>(abs_<ISA, T>(x), src, mask);
}

__RAZE_VX_NAMESPACE_END