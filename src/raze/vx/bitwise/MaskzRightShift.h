#pragma once 

#include <src/raze/vx/bitwise/RightShift.h>
#include <src/raze/vx/arithmetic/MaskzAssign.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Maskz_right_shift;

template <class _Type_>
struct _Maskz_right_shift<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _Type_>()(
			_Right_shift<arch::ISA::SSE2, 128, _Type_>()(__left, __shift), __mask);
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::SSE41, 128, _Type_>:
	_Maskz_right_shift<arch::ISA::SSSE3, 128, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
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
			return _Maskz_right_shift<arch::ISA::SSE2, 128, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX512VLF, 128, _Type_>: 
	_Maskz_right_shift<arch::ISA::AVX2, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 128, _Type_>()(
			_Right_shift<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_sra_epi64(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_srl_epi64(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_sra_epi32(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_srl_epi32(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Right_shift<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX512VLBW, 128, _Type_> :
	_Maskz_right_shift<arch::ISA::AVX512VLF, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _Type_>()(
			_Right_shift<arch::ISA::AVX512VLBW, 128, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_sra_epi16(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_srl_epi16(__mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _Type_>()(
				_Right_shift<arch::ISA::AVX512VLBW, 128, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX, 256, _Type_>()(
			_Right_shift<arch::ISA::AVX, 256, _Type_>()(__left, __shift), __mask);
	}
};

template <class _Type_>
struct _Maskz_right_shift<arch::ISA::AVX2, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _Type_>()(
			_Right_shift<arch::ISA::AVX2, 256, _Type_>()(__left, __shift), __mask);
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX512VLF, 256, _Type_>: 
	_Maskz_right_shift<arch::ISA::AVX2, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _Type_>()(
			_Right_shift<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_sra_epi64(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_srl_epi64(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_sra_epi32(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_srl_epi32(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLF, 256, _Type_>()(
				_Right_shift<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX512VLBW, 256, _Type_>: 
	_Maskz_right_shift<arch::ISA::AVX512VLF, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _Type_>()(
			_Right_shift<arch::ISA::AVX512VLBW, 256, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_sra_epi16(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_srl_epi16(__mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _Type_>()(
				_Right_shift<arch::ISA::AVX512VLBW, 256, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_>
struct _Maskz_right_shift<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512F, 512, _Type_>()(
			_Right_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_sra_epi64(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_srl_epi64(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_sra_epi32(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_srl_epi32(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512F, 512, _Type_>()(
				_Right_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_> 
struct _Maskz_right_shift<arch::ISA::AVX512BW, 512, _Type_> :
	_Maskz_right_shift<arch::ISA::AVX512F, 512, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512BW, 512, _Type_>()(
			_Right_shift<arch::ISA::AVX512BW, 512, _Type_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_sra_epi16(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_srl_epi16(__mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512BW, 512, _Type_>()(
				_Right_shift<arch::ISA::AVX512BW, 512, _Type_>()(__left, __shift), __mask);
		}
	}
};

template <class _Type_> struct _Maskz_right_shift<arch::ISA::SSE3, 128, _Type_> : _Maskz_right_shift<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::SSSE3, 128, _Type_> : _Maskz_right_shift<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::SSE42, 128, _Type_> : _Maskz_right_shift<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX, 128, _Type_> : _Maskz_right_shift<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::FMA3, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX2, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX2FMA3, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512DQ, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512BWDQ, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Maskz_right_shift<arch::ISA::FMA3, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX2FMA3, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VLDQ, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VLDQ, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Maskz_right_shift<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Maskz_right_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
