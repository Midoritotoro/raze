#pragma once 

#include <src/raze/datapar/bitwise/ToMask.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_to_index_mask;

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm_movemask_epi8(__intrin_bitcast<__m128i>(__vector));
		
		else
			return _Simd_to_mask<arch::ISA::SSE2, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm256_movemask_epi8(__intrin_bitcast<__m256i>(__vector));
		
		else
			return _Simd_to_mask<arch::ISA::AVX2, 256, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>) {
			return __vector;
		}
		else if constexpr (sizeof(_DesiredType_) == 8) {
			return _mm512_cmp_epi64_mask(__intrin_bitcast<__m512i>(__vector), _mm512_setzero_si512(), _MM_CMPINT_LT);
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return _mm512_cmp_epi32_mask(__intrin_bitcast<__m512i>(__vector), _mm512_setzero_si512(), _MM_CMPINT_LT);
		}
		else {
			constexpr auto __ymm_bits = (sizeof(_IntrinType_) / sizeof(_DesiredType_)) >> 1;

			const auto __low = _Simd_to_index_mask<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256d>(__vector));
			const auto __high = _Simd_to_index_mask<arch::ISA::AVX2, 256, _DesiredType_>()(_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1));

			using _Type = IntegerForSize<sizeof(decltype(__low)) * 2>::Unsigned;

			return (static_cast<_Type>(__high) << __ymm_bits) | static_cast<_Type>(__low);
		}
	}
};

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::AVX512BW, 512, _DesiredType_>: 
	_Simd_to_index_mask<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;
		else
			return _mm512_movepi8_mask(__intrin_bitcast<__m512i>(__vector));
	}
};

template <class _DesiredType_> 
struct _Simd_to_index_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
	_Simd_to_index_mask<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;
		else
			return _Simd_to_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> 
struct _Simd_to_index_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_Simd_to_index_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;
		else
			return _Simd_to_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>:
	_Simd_to_index_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;
		else if constexpr (sizeof(_DesiredType_) <= 2)
			return _Simd_to_index_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__vector);
		else
			return _Simd_to_index_mask<arch::ISA::AVX512VLDQ, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _Simd_to_index_mask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>:
	_Simd_to_index_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;
		else if constexpr (sizeof(_DesiredType_) <= 2)
			return _Simd_to_index_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__vector);
		else
			return _Simd_to_index_mask<arch::ISA::AVX512VLDQ, 256, _DesiredType_>()(__vector);
	}
};


template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_to_index_mask<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_to_index_mask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
