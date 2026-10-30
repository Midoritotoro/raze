#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <raze/math/BitCast.h>
#include <src/raze/vx/hw/x86/merge/Select.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class T>
concept has_value_type = requires { typename T::value_type; };

template <class T, class = void>
struct unwrap_int {
	using type = T;
};

template <class T>
struct unwrap_int<T, std::void_t<typename T::value_type>> {
	using type = typename T::value_type;
};

template <arch::ISA	ISA, intrin_or_arithmetic_type V, class WrappedType>
raze_always_inline V broadcast_(WrappedType value) noexcept
	requires(arithmetic_type<WrappedType> || has_value_type<WrappedType>)
{
	using T = typename unwrap_int<WrappedType>::type;

	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastq_epi64(_mm_cvtsi64_si128(math::pointer_to_integral(value))));
			else return as<V>(_mm_set1_epi64x(math::pointer_to_integral(value)));
		}
		else if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastd_epi32(_mm_cvtsi32_si128(math::pointer_to_integral(value))));
			else return as<V>(_mm_set1_epi32(math::pointer_to_integral(value)));
		}
		else if constexpr (epi16<T> || epu16<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastw_epi16(_mm_cvtsi32_si128(value)));
			else return as<V>(_mm_set1_epi16(value));
		}
		else if constexpr (epi8<T> || epu8<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastb_epi8(_mm_cvtsi32_si128(value)));
			else return as<V>(_mm_set1_epi8(value));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastss_ps(_mm_set_ss(value)));
			else return as<V>(_mm_set1_ps(value));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm_broadcastsd_pd(_mm_set_sd(value)));
			else return as<V>(_mm_set1_pd(value));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(math::pointer_to_integral(value))));
			else return as<V>(_mm256_set1_epi64x(math::pointer_to_integral(value)));
		}
		else if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastd_epi32(_mm_cvtsi32_si128(math::pointer_to_integral(value))));
			else return as<V>(_mm256_set1_epi32(math::pointer_to_integral(value)));
		}
		else if constexpr (epi16<T> || epu16<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastw_epi16(_mm_cvtsi32_si128(value)));
			else return as<V>(_mm256_set1_epi16(value));
		}
		else if constexpr (epi8<T> || epu8<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastb_epi8(_mm_cvtsi32_si128(value)));
			else return as<V>(_mm256_set1_epi8(value));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastss_ps(_mm_set_ss(value)));
			else return as<V>(_mm256_set1_ps(value));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx2<ISA>) return as<V>(_mm256_broadcastsd_pd(_mm_set_sd(value)));
			else return as<V>(_mm256_set1_pd(value));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_set1_epi64(math::pointer_to_integral(value))); 
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_set1_epi32(math::pointer_to_integral(value)));
		else if constexpr (epi16<T> || epu16<T>) {
			if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_set1_epi16(value));
			else {
				u32 x = static_cast<u16>(value);
				x |= x << 16;
				return as<V>(_mm512_set1_epi32(static_cast<i32>(x)));
			}
		}
		else if constexpr (epi8<T> || epu8<T>) {
			if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_set1_epi8(value));
			else return _mm512_set1_epi32(static_cast<u8>(value) * 0x01010101u);
		}
		else if constexpr (ps<T>) return as<V>(_mm512_set1_ps(value));
		else if constexpr (pd<T>) return as<V>(_mm512_set1_pd(value));
	}
	else {
		return value;
	}
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, class WrappedType, raw_mask_type Mask>
raze_always_inline V broadcast_(WrappedType value, Mask mask) noexcept
	requires(arithmetic_type<WrappedType> || has_value_type<WrappedType>)
{
	using T = typename unwrap_int<WrappedType>::type;

	if constexpr (sizeof(V) == 16) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_maskz_broadcastq_epi64(mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_maskz_broadcastd_epi32(mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm_maskz_broadcastw_epi16(mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm_maskz_broadcastb_epi8(mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_maskz_broadcastss_ps(mask, _mm_set_ss(value)));
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastq_epi64(mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastd_epi32(mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastw_epi16(mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastb_epi8(mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastss_ps(mask, _mm_set_ss(value)));
		else if constexpr (pd<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_broadcastsd_pd(mask, _mm_set_sd(value)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_broadcastq_epi64(mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_broadcastd_epi32(mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA>) return as<V>(_mm512_maskz_broadcastw_epi16(mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA>) return as<V>(_mm512_maskz_broadcastb_epi8(mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_broadcastss_ps(mask, _mm_set_ss(value)));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_broadcastsd_pd(mask, _mm_set_sd(value)));
	}

	return select_<ISA, T>(broadcast_<ISA, V>(value), mask);
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, class WrappedType, raw_mask_type Mask, intrin_or_arithmetic_type Src>
raze_always_inline V broadcast_(WrappedType value, Mask mask, Src src) noexcept
	requires(arithmetic_type<WrappedType> || has_value_type<WrappedType>)
{
	using T = typename unwrap_int<WrappedType>::type;

	if constexpr (sizeof(V) == 16) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mask_broadcastq_epi64(as<__m128i>(src), mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mask_broadcastd_epi32(as<__m128i>(src), mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mask_broadcastw_epi16(as<__m128i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mask_broadcastb_epi8(as<__m128i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mask_broadcastss_ps(as<__m128>(src), mask, _mm_set_ss(value)));
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastq_epi64(as<__m256i>(src), mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastd_epi32(as<__m256i>(src), mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastw_epi16(as<__m256i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastb_epi8(as<__m256i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastss_ps(as<__m256>(src), mask, _mm_set_ss(value)));
		else if constexpr (pd<T> && has_avx512f<ISA> && has_avx512vl<ISA>) return as<V>(_mm256_mask_broadcastsd_pd(as<__m256d>(src), mask, _mm_set_sd(value)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_broadcastq_epi64(as<__m512i>(src), mask, _mm_cvtsi64_si128(math::pointer_to_integral(value))));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_broadcastd_epi32(as<__m512i>(src), mask, _mm_cvtsi32_si128(math::pointer_to_integral(value))));
		else if constexpr ((epi16<T> || epu16<T>) && has_avx512bw<ISA>) return as<V>(_mm512_mask_broadcastw_epi16(as<__m512i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr ((epi8<T> || epu8<T>) && has_avx512bw<ISA>) return as<V>(_mm512_mask_broadcastb_epi8(as<__m512i>(src), mask, _mm_cvtsi32_si128(value)));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_broadcastss_ps(as<__m512>(src), mask, _mm_set_ss(value)));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_broadcastsd_pd(as<__m512d>(src), mask, _mm_set_sd(value)));
	}

	return select_<ISA, T>(broadcast_<ISA, V>(value), src, mask);
}

__RAZE_VX_NAMESPACE_END
