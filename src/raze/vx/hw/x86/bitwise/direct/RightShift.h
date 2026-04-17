#pragma once 

#include <src/raze/vx/shuffle/Blend.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Right_shift;

template <class _Type_>
struct _Right_shift<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			auto __high_shifted = _mm_setzero_si128();
			auto __low_shifted = _mm_setzero_si128();

			if (__shift <= 32) {
				const auto __shift_vector = _mm_cvtsi32_si128(__shift);
				__high_shifted = _mm_sra_epi32(__as<__m128i>(__left), __shift_vector);
				__low_shifted = _mm_srl_epi64(__as<__m128i>(__left), __shift_vector);
			}
			else {
				const auto __shift_vector = _mm_cvtsi32_si128(static_cast<int32>(__shift) - 32);
				__high_shifted = _mm_srai_epi32(__as<__m128i>(__left), 31);
				__low_shifted = _mm_srli_epi64(_mm_sra_epi32(__as<__m128i>(__left), __shift_vector), 32);
			}

			return __as<_IntrinType_>(_Blend<arch::ISA::SSE2, 128, _Type_>()(
				__high_shifted, __low_shifted, _mm_set_epi32(-1, 0, -1, 0)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_srl_epi64(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sra_epi32(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_srl_epi32(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sra_epi16(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_srl_epi16(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			const auto __select_mask = _mm_set1_epi32(0x00FF00FF);
	
			const auto __even_shifted = _mm_sra_epi16(
				_mm_slli_epi16(__as<__m128i>(__left), 8), _mm_cvtsi32_si128(__shift + 8));
			const auto __odd_shifted = _mm_sra_epi16(__as<__m128i>(__left), _mm_cvtsi32_si128(__shift));

			return __as<_IntrinType_>(_Blend<arch::ISA::SSE2, 128, _Type_>()(
				__even_shifted, __odd_shifted, __select_mask));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			const auto __and_mask = _mm_and_si128(__as<__m128i>(__left), _mm_set1_epi8(0xFFu << __shift));
			return __as<_IntrinType_>(_mm_srl_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
	}
};

template <class _Type_> 
struct _Right_shift<arch::ISA::SSE41, 128, _Type_>:
	_Right_shift<arch::ISA::SSSE3, 128, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			auto __high_shifted = _mm_setzero_si128();
			auto __low_shifted = _mm_setzero_si128();

			if (__shift <= 32) {
				const auto __shift_vector = _mm_cvtsi32_si128(__shift);
				__high_shifted = _mm_sra_epi32(__as<__m128i>(__left), __shift_vector);
				__low_shifted = _mm_srl_epi64(__as<__m128i>(__left), __shift_vector);
			}
			else {
				const auto __shift_vector = _mm_cvtsi32_si128(static_cast<int32>(__shift) - 32);
				__high_shifted = _mm_srai_epi32(__as<__m128i>(__left), 31);
				__low_shifted = _mm_srli_epi64(_mm_sra_epi32(__as<__m128i>(__left), __shift_vector), 32);
			}

			return __as<_IntrinType_>(_mm_blend_epi16(__low_shifted, __high_shifted, 0xCC));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			const auto __select_mask = _mm_set1_epi32(0x00FF00FF);

			const auto __even_shifted = _mm_sra_epi16(
				_mm_slli_epi16(__as<__m128i>(__left), 8), _mm_cvtsi32_si128(__shift + 8));
			const auto __odd_shifted = _mm_sra_epi16(__as<__m128i>(__left), _mm_cvtsi32_si128(__shift));

			return __as<_IntrinType_>(_Blend<arch::ISA::SSE41, 128, _Type_>()(
				__even_shifted, __odd_shifted, __select_mask));
		}
		else {
			return _Right_shift<arch::ISA::SSE2, 128, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_> 
struct _Right_shift<arch::ISA::AVX512VLF, 128, _Type_>: 
	_Right_shift<arch::ISA::AVX2, 128, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sra_epi64(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Right_shift<arch::ISA::SSE41, 128, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_> 
struct _Right_shift<arch::ISA::AVX512VLBW, 128, _Type_> :
	_Right_shift<arch::ISA::AVX512VLF, 128, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi8_v<_Type_>) {
			const auto __even_shifted = _mm_sra_epi16(
				_mm_slli_epi16(__as<__m128i>(__left), 8), _mm_cvtsi32_si128(__shift + 8));
			const auto __odd_shifted = _mm_sra_epi16(__as<__m128i>(__left), _mm_cvtsi32_si128(__shift));

			return __as<_IntrinType_>(_mm_mask_mov_epi8(__odd_shifted, 0x5555, __even_shifted));
		}
		else {
			return _Right_shift<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_>
struct _Right_shift<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		const auto __low = _Right_shift<arch::ISA::SSE42, 128, _Type_>()(
			__as<__m128i>(__left), __shift);
		const auto __high = _Right_shift<arch::ISA::SSE42, 128, _Type_>()(
			_mm256_extractf128_si256(__as<__m256i>(__left), 1), __shift);

		return __as<_IntrinType_>(_mm256_insertf128_si256(
			__as<__m256i>(__low), __high, 1));
	}
};

template <class _Type_>
struct _Right_shift<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			auto __high_shifted = _mm256_setzero_si256();
			auto __low_shifted = _mm256_setzero_si256();

			if (__shift <= 32) {
				const auto __shift_vector = _mm_cvtsi32_si128(__shift);
				__high_shifted = _mm256_sra_epi32(__as<__m256i>(__left), __shift_vector);
				__low_shifted = _mm256_srl_epi64(__as<__m256i>(__left), __shift_vector);
			}
			else {
				__high_shifted = _mm256_srai_epi32(__as<__m256i>(__left), 31);
				__low_shifted = _mm256_srli_epi64(_mm256_sra_epi32(
					__as<__m256i>(__left), _mm_cvtsi32_si128(__shift - 32)), 32);
			}

			return __as<_IntrinType_>(_mm256_blend_epi32(__low_shifted, __high_shifted, 0xAA));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_srl_epi64(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sra_epi32(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_srl_epi32(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sra_epi16(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_srl_epi16(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			const auto __select_mask = _mm256_set1_epi32(0x00FF00FF);

			const auto __even_shifted = _mm256_sra_epi16(
				_mm256_slli_epi16(__as<__m256i>(__left), 8), _mm_cvtsi32_si128(__shift + 8));
			const auto __odd_shifted = _mm256_sra_epi16(__as<__m256i>(__left), _mm_cvtsi32_si128(__shift));

			return __as<_IntrinType_>(_Blend<arch::ISA::AVX2, 256, _Type_>()(
				__even_shifted, __odd_shifted, __select_mask));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			const auto __and_mask = _mm256_and_si256(__as<__m256i>(__left), _mm256_set1_epi8(0xFFu << __shift));
			return __as<_IntrinType_>(_mm256_srl_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
	}
};

template <class _Type_> 
struct _Right_shift<arch::ISA::AVX512VLF, 256, _Type_>: 
	_Right_shift<arch::ISA::AVX2, 256, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sra_epi64(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Right_shift<arch::ISA::AVX2, 256, _Type_>()(__left, __shift);
		}
	}
};


template <class _Type_>
struct _Right_shift<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sra_epi64(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_srl_epi64(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sra_epi32(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_srl_epi32(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			const auto __low = __as<__m256i>(__left);
			const auto __high = _mm512_extracti64x4_epi64(__as<__m512i>(__left), 1);

			const auto __low_shifted = _Right_shift<arch::ISA::AVX2, 256, _Type_>()(__low, __shift);
			const auto __high_shifted = _Right_shift<arch::ISA::AVX2, 256, _Type_>()(__high, __shift);

			return __as<_IntrinType_>(_mm512_inserti64x4(__as<__m512i>(__low_shifted), __high_shifted, 1));
		}
	}
};

template <class _Type_> 
struct _Right_shift<arch::ISA::AVX512BW, 512, _Type_> :
	_Right_shift<arch::ISA::AVX512F, 512, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sra_epi16(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_srl_epi16(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			const auto __even_vector = _mm512_slli_epi16(__as<__m512i>(__left), 8);

			const auto __even_shifted = _mm512_sra_epi16(__even_vector, _mm_cvtsi32_si128(__shift + 8));
			const auto __odd_shifted = _mm512_sra_epi16(__as<__m512i>(__left), _mm_cvtsi32_si128(__shift));

			return __as<_IntrinType_>(_mm512_mask_mov_epi8(__even_shifted, 0xAAAAAAAAAAAAAAAA, __odd_shifted));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			const auto __and_mask = _mm512_and_si512(__as<__m512i>(__left), _mm512_set1_epi8(0xFFu << __shift));
			return __as<_IntrinType_>(_mm512_srl_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Right_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_> struct _Right_shift<arch::ISA::SSE3, 128, _Type_> : _Right_shift<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::SSSE3, 128, _Type_> : _Right_shift<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::SSE42, 128, _Type_> : _Right_shift<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX, 128, _Type_> : _Right_shift<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::FMA3, 128, _Type_> : _Right_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX2, 128, _Type_> : _Right_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX2FMA3, 128, _Type_> : _Right_shift<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Right_shift<arch::ISA::AVX512DQ, 512, _Type_> : _Right_shift<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512BWDQ, 512, _Type_> : _Right_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI, 512, _Type_> : _Right_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2, 512, _Type_> : _Right_shift<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Right_shift<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Right_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Right_shift<arch::ISA::FMA3, 256, _Type_> : _Right_shift<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX2FMA3, 256, _Type_> : _Right_shift<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VLBW, 256, _Type_> : _Right_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VLDQ, 256, _Type_> : _Right_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Right_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Right_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Right_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Right_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Right_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Right_shift<arch::ISA::AVX512VLDQ, 128, _Type_> : _Right_shift<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Right_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Right_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Right_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Right_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Right_shift<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Right_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
