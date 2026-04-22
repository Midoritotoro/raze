#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Selectz.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	uint32			_Width_,
	arithmetic_type	_Type_>
struct _Mul;

template <arithmetic_type _Type_>
struct _Mul<arch::ISA::SSE2, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mul_pd(
				__as<__m128d>(__left), __as<__m128d>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mul_ps(
				__as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			const auto __left_high = _mm_srli_epi64(__as<__m128i>(__left), 32);
			const auto __right_high = _mm_srli_epi64(__as<__m128i>(__right), 32);

			const auto __multiplied1 = _mm_mul_epu32(__left_high, __as<__m128i>(__right));
			const auto __multiplied2 = _mm_mul_epu32(__right_high, __as<__m128i>(__left));

			const auto __add64 = _mm_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm_slli_epi64(__add64, 32);
			const auto __low = _mm_mul_epu32(__as<__m128i>(__left), __as<__m128i>(__right));

			return __as<_IntrinType_>(_mm_add_epi64(__low, __high));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			const auto __shuffled_left = _mm_shuffle_epi32(__as<__m128i>(__left), 0xF5);
			const auto __shuffled_right = _mm_shuffle_epi32(__as<__m128i>(__right), 0xF5);

			const auto __product_02 = _mm_mul_epu32(__as<__m128i>(__left), __as<__m128i>(__right));
			const auto __product_13 = _mm_mul_epu32(__shuffled_left, __shuffled_right);

			const auto __product_01 = _mm_unpacklo_epi32(__product_02, __product_13);
			const auto __product_23 = _mm_unpackhi_epi32(__product_02, __product_13);

			return __as<_IntrinType_>(_mm_unpacklo_epi64(__product_01, __product_23));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mullo_epi16(
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __left_odd = _mm_srli_epi16(__as<__m128i>(__left), 8);
			const auto __right_odd = _mm_srli_epi16(__as<__m128i>(__right), 8);

			const auto __multiplied_even = _mm_mullo_epi16(
				__as<__m128i>(__left), __as<__m128i>(__right));
			const auto __multiplied_odd = _mm_slli_epi16(_mm_mullo_epi16(__left_odd, __right_odd), 8);

			return __as<_IntrinType_>(_Select<arch::ISA::SSE2, 128, _Type_>()(
				__multiplied_even, __multiplied_odd, _mm_set1_epi32(0x00FF00FF)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_> struct _Mul<arch::ISA::SSE3, 128, _Type_> : _Mul<arch::ISA::SSE2, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::SSSE3, 128, _Type_> : _Mul<arch::ISA::SSE3, 128, _Type_> {};

template <arithmetic_type _Type_> 
struct _Mul<arch::ISA::SSE41, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Mul<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE41, 128, _Type_>()(
			_Mul<arch::ISA::SSE41, 128, _Type_>()(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE41, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Mul<arch::ISA::AVX, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mul_ps(
				__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mul_pd(
				__as<__m256d>(__left), __as<__m256d>(__right)));
		}
		else {
			const auto __low = _Mul<arch::ISA::SSE42, 128, _Type_>()(
				__as<__m128i>(__left), __as<__m128i>(__right));
			const auto __high = _Mul<arch::ISA::SSE42, 128, _Type_>()(
				_mm256_extractf128_si256(__as<__m256i>(__left), 1),
				_mm256_extractf128_si256(__as<__m256i>(__right), 1));

			return __as<_IntrinType_>(_mm256_insertf128_si256(
				__as<__m256i>(__low), __high, 1));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()(
			_Mul<arch::ISA::AVX, 256, _Type_>()(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Mul<arch::ISA::AVX2, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __left_odd = _mm256_srli_epi16(__as<__m256i>(__left), 8);
			const auto __right_odd = _mm256_srli_epi16(__as<__m256i>(__right), 8);

			const auto __multiplied_even = _mm256_mullo_epi16(
				__as<__m256i>(__left), __as<__m256i>(__right));
			const auto __multiplied_odd = _mm256_slli_epi16(_mm256_mullo_epi16(__left_odd, __right_odd), 8);

			return __as<_IntrinType_>(_Select<arch::ISA::AVX2, 256, _Type_>()(
				__multiplied_even, __multiplied_odd, _mm256_set1_epi32(0x00FF00FF)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mullo_epi16(
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mullo_epi32(
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			const auto __swapped = _mm256_shuffle_epi32(__as<__m256i>(__right), 0xB1);
			const auto __multiplied_32 = _mm256_mullo_epi32(__as<__m256i>(__left), __swapped);

			const auto __add_32 = _mm256_hadd_epi32(__multiplied_32, _mm256_setzero_si256());
			const auto __shuffled_32 = _mm256_shuffle_epi32(__add_32, 0x73);

			const auto __final_32 = _mm256_mul_epu32(__as<__m256i>(__left), __as<__m256i>(__right));
			return __as<_IntrinType_>(_mm256_add_epi64(__final_32, __shuffled_32));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mul_ps(
				__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mul_pd(
				__as<__m256d>(__left), __as<__m256d>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()(
			_Mul<arch::ISA::AVX2, 256, _Type_>()(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Mul<arch::ISA::AVX512F, 512, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			const auto __left_high = _mm512_srli_epi64(__as<__m512i>(__left), 32);
			const auto __right_high = _mm512_srli_epi64(__as<__m512i>(__right), 32);

			const auto __multiplied1 = _mm512_mul_epu32(__left_high, __as<__m512i>(__right));
			const auto __multiplied2 = _mm512_mul_epu32(__right_high, __as<__m512i>(__left));

			const auto __add64 = _mm512_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm512_slli_epi64(__add64, 32);
			const auto __low = _mm512_mul_epu32(__as<__m512i>(__left), __as<__m512i>(__right));

			return __as<_IntrinType_>(_mm512_add_epi64(__low, __high));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mullo_epi32(
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mul_ps(
				__as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mul_pd(
				__as<__m512d>(__left), __as<__m512d>(__right)));
		}
		else {
			const auto __low = _Mul<arch::ISA::AVX2, 256, _Type_>()(
				__as<__m256i>(__left), __as<__m256i>(__right));

			const auto __high = _Mul<arch::ISA::AVX2, 256, _Type_>()(
				_mm512_extractf64x4_pd(__as<__m512d>(__left), 1),
				_mm512_extractf64x4_pd(__as<__m512d>(__right), 1));

			return __as<_IntrinType_>(_mm512_inserti64x4(
				__as<__m512i>(__low), __as<__m256i>(__high), 1));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()(
				_Mul<arch::ISA::AVX512F, 512, _Type_>()(__left, __right), __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_mul_ps(__mask,
				__as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_mul_pd(__mask,
				__as<__m512d>(__left), __as<__m512d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_mullo_epi32(
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm512_srli_epi64(__as<__m512i>(__left), 32);
			const auto __right_high = _mm512_srli_epi64(__as<__m512i>(__right), 32);

			const auto __multiplied1 = _mm512_mul_epu32(__left_high, __as<__m512i>(__right));
			const auto __multiplied2 = _mm512_mul_epu32(__right_high, __as<__m512i>(__left));

			const auto __add64 = _mm512_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm512_slli_epi64(__add64, 32);
			const auto __low = _mm512_mul_epu32(__as<__m512i>(__left), __as<__m512i>(__right));

			return __as<_IntrinType_>(_mm512_maskz_add_epi64(__mask, __low, __high));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Select<arch::ISA::AVX512F, 512, _Type_>()(
				_Mul<arch::ISA::AVX512F, 512, _Type_>()(__left, __right), __source, __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_mul_ps(__as<__m512>(__source),
				__mask, __as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_mul_pd(__as<__m512d>(__source), 
				__mask, __as<__m512d>(__left), __as<__m512d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_mullo_epi32(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm512_srli_epi64(__as<__m512i>(__left), 32);
			const auto __right_high = _mm512_srli_epi64(__as<__m512i>(__right), 32);

			const auto __multiplied1 = _mm512_mul_epu32(__left_high, __as<__m512i>(__right));
			const auto __multiplied2 = _mm512_mul_epu32(__right_high, __as<__m512i>(__left));

			const auto __add64 = _mm512_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm512_slli_epi64(__add64, 32);
			const auto __low = _mm512_mul_epu32(__as<__m512i>(__left), __as<__m512i>(__right));

			return __as<_IntrinType_>(_mm512_mask_add_epi64(__as<__m512i>(__source), __mask, __low, __high));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Mul<arch::ISA::AVX512BW, 512, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mullo_epi16(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else {
			return _Mul<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512F, 512, _Type_>()(__left, __right, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_mullo_epi16(
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512F, 512, _Type_>()(__left, __right, __mask, __source);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_mullo_epi16(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}
};

template <arithmetic_type _Type_>
struct _Mul<arch::ISA::AVX512VLF, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Mul<arch::ISA::SSE41, 128, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Mul<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right), __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_mul_ps(__mask,
				__as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_mul_pd(__mask,
				__as<__m128d>(__left), __as<__m128d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_mullo_epi32(
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm_srli_epi64(__as<__m128i>(__left), 32);
			const auto __right_high = _mm_srli_epi64(__as<__m128i>(__right), 32);

			const auto __multiplied1 = _mm_mul_epu32(__left_high, __as<__m128i>(__right));
			const auto __multiplied2 = _mm_mul_epu32(__right_high, __as<__m128i>(__left));

			const auto __add64 = _mm_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm_slli_epi64(__add64, 32);
			const auto __low = _mm_mul_epu32(__as<__m128i>(__left), __as<__m128i>(__right));

			return __as<_IntrinType_>(_mm_maskz_add_epi64(__mask, __low, __high));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Select<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Mul<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right), __source, __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_mul_ps(__as<__m128>(__source),
				__mask, __as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_mul_pd(__as<__m128d>(__source), 
				__mask, __as<__m128d>(__left), __as<__m128d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_mullo_epi32(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm_srli_epi64(__as<__m128i>(__left), 32);
			const auto __right_high = _mm_srli_epi64(__as<__m128i>(__right), 32);

			const auto __multiplied1 = _mm_mul_epu32(__left_high, __as<__m128i>(__right));
			const auto __multiplied2 = _mm_mul_epu32(__right_high, __as<__m128i>(__left));

			const auto __add64 = _mm_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm_slli_epi64(__add64, 32);
			const auto __low = _mm_mul_epu32(__as<__m128i>(__left), __as<__m128i>(__right));

			return __as<_IntrinType_>(_mm_mask_add_epi64(__as<__m128i>(__source), __mask, __low, __high));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Mul<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mullo_epi16(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else {
			return _Mul<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_mullo_epi16(
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right, __mask, __source);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_mullo_epi16(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}
};

template <arithmetic_type _Type_>
struct _Mul<arch::ISA::AVX512VLF, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Mul<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()(
				_Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right), __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_mul_ps(__mask,
				__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_mul_pd(__mask,
				__as<__m256d>(__left), __as<__m256d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_mullo_epi32(
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm256_srli_epi64(__as<__m256i>(__left), 32);
			const auto __right_high = _mm256_srli_epi64(__as<__m256i>(__right), 32);

			const auto __multiplied1 = _mm256_mul_epu32(__left_high, __as<__m256i>(__right));
			const auto __multiplied2 = _mm256_mul_epu32(__right_high, __as<__m256i>(__left));

			const auto __add64 = _mm256_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm256_slli_epi64(__add64, 32);
			const auto __low = _mm256_mul_epu32(__as<__m256i>(__left), __as<__m256i>(__right));

			return __as<_IntrinType_>(_mm256_maskz_add_epi64(__mask, __low, __high));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) < 4)) {
			return _Select<arch::ISA::AVX512VLF, 256, _Type_>()(
				_Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right), __source, __mask);
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_mul_ps(__as<__m256>(__source),
				__mask, __as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_mul_pd(__as<__m256d>(__source), 
				__mask, __as<__m256d>(__left), __as<__m256d>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_mullo_epi32(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_> || __is_epi64_v<_Type_>) {
			const auto __left_high = _mm256_srli_epi64(__as<__m256i>(__left), 32);
			const auto __right_high = _mm256_srli_epi64(__as<__m256i>(__right), 32);

			const auto __multiplied1 = _mm256_mul_epu32(__left_high, __as<__m256i>(__right));
			const auto __multiplied2 = _mm256_mul_epu32(__right_high, __as<__m256i>(__left));

			const auto __add64 = _mm256_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm256_slli_epi64(__add64, 32);
			const auto __low = _mm256_mul_epu32(__as<__m256i>(__left), __as<__m256i>(__right));

			return __as<_IntrinType_>(_mm256_mask_add_epi64(__as<__m256i>(__source), __mask, __low, __high));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Mul<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_mullo_epi16(
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || (sizeof(_Type_) != 2)) {
			return _Mul<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right, __mask, __source);
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_mullo_epi16(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}
};

template <arithmetic_type _Type_> struct _Mul<arch::ISA::SSE42, 128, _Type_> : _Mul<arch::ISA::SSE41, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX, 128, _Type_> : _Mul<arch::ISA::SSE42, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX2, 128, _Type_> : _Mul<arch::ISA::AVX, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::FMA3, 128, _Type_> : _Mul<arch::ISA::AVX, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX2FMA3, 128, _Type_> : _Mul<arch::ISA::AVX2, 128, _Type_> {};

template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512DQ, 512, _Type_> : _Mul<arch::ISA::AVX512F, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512BWDQ, 512, _Type_> : _Mul<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI, 512, _Type_> : _Mul<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2, 512, _Type_> : _Mul<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Mul<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Mul<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <arithmetic_type _Type_> struct _Mul<arch::ISA::FMA3, 256, _Type_> : _Mul<arch::ISA::AVX, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX2FMA3, 256, _Type_> : _Mul<arch::ISA::AVX2, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VLDQ, 256, _Type_> : _Mul<arch::ISA::AVX512VLF, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Mul<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Mul<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Mul<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Mul<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Mul<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VLDQ, 128, _Type_> : _Mul<arch::ISA::AVX512VLF, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Mul<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Mul<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Mul<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Mul<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Mul<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Mul<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END