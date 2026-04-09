#pragma once 

#include <src/raze/datapar/bitwise/ToMask.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _To_bitmask;

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 2)
			return static_cast<uint16>(_mm_movemask_epi8(__intrin_bitcast<__m128i>(__vector)));
		
		else
			return _To_mask<arch::ISA::SSE2, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>) {
			return __vector;
		}
		else if constexpr (sizeof(_DesiredType_) >= 4) {
			return _To_mask<arch::ISA::AVX, 256, _DesiredType_>()(__vector);
		}
		else {
			const auto __low = _To_bitmask<arch::ISA::SSE42, 128, _DesiredType_>()(__intrin_bitcast<__m128i>(__vector));
			const auto __high = _To_bitmask<arch::ISA::SSE42, 128, _DesiredType_>()(
				_mm256_extractf128_si256(__intrin_bitcast<__m256i>(__vector), 1));

			return (static_cast<uint32>(__high) << 16) | static_cast<uint32>(__low);
		}
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 2)
			return static_cast<uint32>(_mm256_movemask_epi8(__intrin_bitcast<__m256i>(__vector)));
		
		else
			return _To_mask<arch::ISA::AVX2, 256, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>) {
			return __vector;
		}
		else if constexpr (sizeof(_DesiredType_) == 8 || sizeof(_DesiredType_) == 4) {
			return _To_mask<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector);
		}
		else {
			const auto __low = _To_bitmask<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256i>(__vector));
			const auto __high = _To_bitmask<arch::ISA::AVX2, 256, _DesiredType_>()(_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1));

			return (static_cast<uint64>(__high) << 32) | static_cast<uint64>(__low);
		}
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512BW, 512, _DesiredType_>: 
	_To_bitmask<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 4 || sizeof(_DesiredType_) == 8)
			return _To_bitmask<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm512_movepi16_mask(__intrin_bitcast<__m512i>(__vector));

		else
			return _mm512_movepi8_mask(__intrin_bitcast<__m512i>(__vector));
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512DQ, 512, _DesiredType_>:
	_To_bitmask<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 8)
			return _mm512_movepi64_mask(__intrin_bitcast<__m512i>(__vector));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return _mm512_movepi32_mask(__intrin_bitcast<__m512i>(__vector));

		else 
			return _To_bitmask<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: 
	_To_bitmask<arch::ISA::AVX512BW, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _To_bitmask<arch::ISA::AVX512DQ, 512, _DesiredType_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512BW, 512, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> 
struct _To_bitmask<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
	_To_bitmask<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 4 || sizeof(_DesiredType_) == 8)
			return _To_bitmask<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm256_movepi16_mask(__intrin_bitcast<__m256i>(__vector));

		else
			return _mm256_movepi8_mask(__intrin_bitcast<__m256i>(__vector));
	}
};


template <class _DesiredType_> 
struct _To_bitmask<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: 
	_To_bitmask<arch::ISA::AVX512VLF, 256, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 8)
			return _mm256_movepi64_mask(__intrin_bitcast<__m256i>(__vector));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return _mm256_movepi32_mask(__intrin_bitcast<__m256i>(__vector));

		else
			return _To_bitmask<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector);
	}
};


template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> :
	_To_bitmask<arch::ISA::AVX512VLBW, 256, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _To_bitmask<arch::ISA::AVX512VLDQ, 256, _DesiredType_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> 
struct _To_bitmask<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_To_bitmask<arch::ISA::AVX512VLF, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 4 || sizeof(_DesiredType_) == 8)
			return _To_bitmask<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm_movepi16_mask(__intrin_bitcast<__m128i>(__vector));

		else
			return _mm_movepi8_mask(__intrin_bitcast<__m128i>(__vector));
	}
};

template <class _DesiredType_> 
struct _To_bitmask<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: 
	_To_bitmask<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_DesiredType_) == 8)
			return _mm_movepi64_mask(__intrin_bitcast<__m128i>(__vector));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return _mm_movepi32_mask(__intrin_bitcast<__m128i>(__vector));

		else
			return _To_bitmask<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector);
	}
};


template <class _DesiredType_>
struct _To_bitmask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>:
	_To_bitmask<arch::ISA::AVX512VLBW, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _To_bitmask<arch::ISA::AVX512VLDQ, 128, _DesiredType_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> struct _To_bitmask<arch::ISA::SSE3, 128, _DesiredType_> : _To_bitmask<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::SSSE3, 128, _DesiredType_> : _To_bitmask<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::SSE41, 128, _DesiredType_> : _To_bitmask<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::SSE42, 128, _DesiredType_> : _To_bitmask<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX, 128, _DesiredType_> : _To_bitmask<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX2, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::FMA3, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX2, 128, _DesiredType_> {};

template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _To_bitmask<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _To_bitmask<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _To_bitmask<arch::ISA::FMA3, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VLF, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VLF, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _To_bitmask<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
