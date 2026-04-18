#pragma once 

#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Selectz;

template <class _Type_>
struct _Selectz<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (std::is_integral_v<_MaskType_>)
			return _Select<arch::ISA::SSE2, 128, _Type_>()(__vector,
				_Zero<arch::ISA::SSE2, 128, _IntrinType_>()(), __mask);
		else
			return _And<arch::ISA::SSE2, 128, _Type_>()(__as<_IntrinType_>(__mask), __vector);
	}
};

template <class _Type_> struct _Selectz<arch::ISA::SSE3, 128, _Type_> : _Selectz<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::SSSE3, 128, _Type_> : _Selectz<arch::ISA::SSE3, 128, _Type_> {};

template <class _Type_>
struct _Selectz<arch::ISA::SSE41, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (std::is_integral_v<_MaskType_>)
			return _Select<arch::ISA::SSE41, 128, _Type_>()(__vector,
				_Zero<arch::ISA::SSE41, 128, _IntrinType_>()(), __mask);
		else
			return _And<arch::ISA::SSE41, 128, _Type_>()(__as<_IntrinType_>(__mask), __vector);
	}
};

template <class _Type_> struct _Selectz<arch::ISA::SSE42, 128, _Type_> : _Selectz<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX, 128, _Type_> : _Selectz<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX2, 128, _Type_> : _Selectz<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::FMA3, 128, _Type_> : _Selectz<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX2FMA3, 128, _Type_> : _Selectz<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> 
struct _Selectz<arch::ISA::AVX, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (std::is_integral_v<_MaskType_>)
			return _Select<arch::ISA::AVX, 256, _Type_>()(__vector,
				_Zero<arch::ISA::AVX, 256, _IntrinType_>()(), __mask);
		else
			return _And<arch::ISA::AVX, 256, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}
};

template <class _Type_>
struct _Selectz<arch::ISA::AVX2, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (std::is_integral_v<_MaskType_>)
			return _Select<arch::ISA::AVX2, 256, _Type_>()(__vector,
				_Zero<arch::ISA::AVX2, 256, _IntrinType_>()(), __mask);
		else
			return _And<arch::ISA::AVX2, 256, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}
};

template <class _Type_>
struct _Selectz<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512F, 512, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_epi64(
				__mask, __as<__m512i>(__vector)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_epi32(
				__mask, __as<__m512i>(__vector)));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_ps(
				__mask, __as<__m512>(__vector)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_pd(
				__mask, __as<__m512d>(__vector)));

		else
			return _Select<arch::ISA::AVX512F, 512, _Type_>()(__vector,
				_Zero<arch::ISA::AVX512F, 512, _IntrinType_>()(), __mask);
	}
};

template <class _Type_>
struct _Selectz<arch::ISA::AVX512BW, 512, _Type_> :
	_Selectz<arch::ISA::AVX512F, 512, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512F, 512, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_epi16(
				__mask, __as<__m512i>(__vector)));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_mov_epi8(
				__mask, __as<__m512i>(__vector)));

		else
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()(__vector, __mask);
	}
};

template <class _Type_>
struct _Selectz<arch::ISA::AVX512VLF, 256, _Type_> :
	_Selectz<arch::ISA::AVX2, 256, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512VLF, 256, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_epi64(
				__mask, __as<__m256i>(__vector)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_epi32(
				__mask, __as<__m256i>(__vector)));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_ps(
				__mask, __as<__m256>(__vector)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_pd(
				__mask, __as<__m256d>(__vector)));

		else
			return _Select<arch::ISA::AVX512VLF, 256, _Type_>()(__vector,
				_Zero<arch::ISA::AVX512VLF, 256, _IntrinType_>()(), __mask);
	}
};

template <class _Type_> 
struct _Selectz<arch::ISA::AVX512VLBW, 256, _Type_>:
	_Selectz<arch::ISA::AVX512VLF, 256, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512VLBW, 256, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_epi16(
				__mask, __as<__m256i>(__vector)));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskz_mov_epi8(
				__mask, __as<__m256i>(__vector)));

		else
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()(__vector, __mask);
	}
};

template <class _Type_>
struct _Selectz<arch::ISA::AVX512VLF, 128, _Type_> :
	_Selectz<arch::ISA::AVX2, 128, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512VLF, 128, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_epi64(
				__mask, __as<__m128i>(__vector)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_epi32(
				__mask, __as<__m128i>(__vector)));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_ps(
				__mask, __as<__m128>(__vector)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_pd(
				__mask, __as<__m128d>(__vector)));

		else
			return _Select<arch::ISA::AVX512VLF, 128, _Type_>()(__vector,
				_Zero<arch::ISA::AVX512VLF, 128, _IntrinType_>()(), __mask);
	}
};

template <class _Type_> 
struct _Selectz<arch::ISA::AVX512VLBW, 128, _Type_>:
	_Selectz<arch::ISA::AVX512VLF, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _And<arch::ISA::AVX512VLBW, 128, _Type_>()(__vector, __as<_IntrinType_>(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_epi16(
				__mask, __as<__m128i>(__vector)));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskz_mov_epi8(
				__mask, __as<__m128i>(__vector)));

		else
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()(__vector, __mask);
	}
};

template <class _Type_> struct _Selectz<arch::ISA::AVX512DQ, 512, _Type_> : _Selectz<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512BWDQ, 512, _Type_> : _Selectz<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI, 512, _Type_> : _Selectz<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2, 512, _Type_> : _Selectz<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Selectz<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Selectz<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Selectz<arch::ISA::FMA3, 256, _Type_> : _Selectz<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX2FMA3, 256, _Type_> : _Selectz<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VLDQ, 256, _Type_> : _Selectz<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Selectz<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Selectz<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Selectz<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Selectz<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Selectz<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Selectz<arch::ISA::AVX512VLDQ, 128, _Type_> : _Selectz<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Selectz<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Selectz<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Selectz<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Selectz<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Selectz<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Selectz<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
