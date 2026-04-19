#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Not;

template <class _Type_>
struct _Not<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept 
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m128d>)
			return _mm_xor_pd(__vector, __as<__m128d>(_mm_set1_epi32(-1)));

		else if constexpr (std::is_same_v<_IntrinType_, __m128i>)
			return _mm_xor_si128(__vector, _mm_set1_epi32(-1));

		else if constexpr (std::is_same_v<_IntrinType_, __m128>)
			return _mm_xor_ps(__vector, __as<__m128>(_mm_set1_epi32(-1)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> struct _Not<arch::ISA::SSE3, 128, _Type_> : _Not<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::SSSE3, 128, _Type_> : _Not<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::SSE41, 128, _Type_> : _Not<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::SSE42, 128, _Type_> : _Not<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX, 128, _Type_> : _Not<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::FMA3, 128, _Type_> : _Not<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX2, 128, _Type_> : _Not<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX2FMA3, 128, _Type_> : _Not<arch::ISA::AVX2, 128, _Type_> {};


template <class _Type_> 
struct _Not<arch::ISA::AVX512VLF, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept
	{
		return __as<_IntrinType_>(_mm_ternarylogic_epi32(
			__as<__m128i>(__vector), __as<__m128i>(__vector),
			__as<__m128i>(__vector), 0x55));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm_maskz_ternarylogic_epi64(
				__mask, __as<__m128i>(__vector),
				__as<__m128i>(__vector), __as<__m128i>(__vector), 0x55));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm_maskz_ternarylogic_epi32(
				__mask, __as<__m128i>(__vector),
				__as<__m128i>(__vector), __as<__m128i>(__vector), 0x55));
		}
		else {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__vector), __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> 
struct _Not<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_xor_pd(__vector, __as<__m256d>(_mm256_set1_epi32(-1)));

		else
			return __as<_IntrinType_>(_mm256_xor_ps(
				__as<__m256>(__vector),
				__as<__m256>(_mm256_set1_epi32(-1))));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_>
struct _Not<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept 
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_xor_pd(__vector, __as<__m256d>(_mm256_set1_epi32(-1)));

		else if constexpr (std::is_same_v<_IntrinType_, __m256i>)
			return _mm256_xor_si256(__vector, _mm256_set1_epi32(-1));

		else if constexpr (std::is_same_v<_IntrinType_, __m256>)
			return _mm256_xor_ps(__vector, __as<__m256>(_mm256_set1_epi32(-1)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> struct _Not<arch::ISA::FMA3, 256, _Type_> : _Not<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX2FMA3, 256, _Type_> : _Not<arch::ISA::AVX2, 256, _Type_> {};

template <class _Type_> 
struct _Not<arch::ISA::AVX512VLF, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept
	{
		return __as<_IntrinType_>(_mm256_ternarylogic_epi32(
			__as<__m256i>(__vector), __as<__m256i>(__vector),
			__as<__m256i>(__vector), 0x55));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm256_maskz_ternarylogic_epi64(
				__mask, __as<__m256i>(__vector),
				__as<__m256i>(__vector), __as<__m256i>(__vector), 0x55));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm256_maskz_ternarylogic_epi32(
				__mask, __as<__m256i>(__vector),
				__as<__m256i>(__vector), __as<__m256i>(__vector), 0x55));
		}
		else {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__vector), __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__vector), __source, __mask);
	}
};


template <class _Type_>
struct _Not<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept 
	{
		return __as<_IntrinType_>(_mm512_ternarylogic_epi32(
			__as<__m512i>(__vector), __as<__m512i>(__vector),
			__as<__m512i>(__vector), 0x55));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm512_maskz_ternarylogic_epi64(
				__mask, __as<__m512i>(__vector),
				__as<__m512i>(__vector), __as<__m512i>(__vector), 0x55));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm512_maskz_ternarylogic_epi32(
				__mask, __as<__m512i>(__vector),
				__as<__m512i>(__vector), __as<__m512i>(__vector), 0x55));
		}
		else {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__vector), __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512F, 512, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> struct _Not<arch::ISA::AVX512BW, 512, _Type_> : _Not<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512DQ, 512, _Type_> : _Not<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512BWDQ, 512, _Type_> : _Not<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI, 512, _Type_> : _Not<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2, 512, _Type_> : _Not<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Not<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Not<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Not<arch::ISA::AVX512VLBW, 256, _Type_> : _Not<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VLDQ, 256, _Type_> : _Not<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Not<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Not<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Not<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Not<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Not<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Not<arch::ISA::AVX512VLBW, 128, _Type_> : _Not<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VLDQ, 128, _Type_> : _Not<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Not<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Not<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Not<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Not<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Not<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Not<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
