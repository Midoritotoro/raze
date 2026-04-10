#pragma once 

#include <src/raze/vx/bitwise/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _To_bitmask;

template <class _Type_>
struct _To_bitmask<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 2)
			return static_cast<uint16>(_mm_movemask_epi8(__as<__m128i>(__vector)));
		
		else
			return _To_mask<arch::ISA::SSE2, 128, _Type_>()(__vector);
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>) {
			return __vector;
		}
		else if constexpr (sizeof(_Type_) >= 4) {
			return _To_mask<arch::ISA::AVX, 256, _Type_>()(__vector);
		}
		else {
			const auto __low = _To_bitmask<arch::ISA::SSE42, 128, _Type_>()(__as<__m128i>(__vector));
			const auto __high = _To_bitmask<arch::ISA::SSE42, 128, _Type_>()(
				_mm256_extractf128_si256(__as<__m256i>(__vector), 1));

			return (static_cast<uint32>(__high) << 16) | static_cast<uint32>(__low);
		}
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 2)
			return static_cast<uint32>(_mm256_movemask_epi8(__as<__m256i>(__vector)));
		
		else
			return _To_mask<arch::ISA::AVX2, 256, _Type_>()(__vector);
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>) {
			return __vector;
		}
		else if constexpr (sizeof(_Type_) == 8 || sizeof(_Type_) == 4) {
			return _To_mask<arch::ISA::AVX512F, 512, _Type_>()(__vector);
		}
		else {
			const auto __low = _To_bitmask<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector));
			const auto __high = _To_bitmask<arch::ISA::AVX2, 256, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__vector), 1));

			return (static_cast<uint64>(__high) << 32) | static_cast<uint64>(__low);
		}
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512BW, 512, _Type_>: 
	_To_bitmask<arch::ISA::AVX512F, 512, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 4 || sizeof(_Type_) == 8)
			return _To_bitmask<arch::ISA::AVX512F, 512, _Type_>()(__vector);

		else if constexpr (sizeof(_Type_) == 2)
			return _mm512_movepi16_mask(__as<__m512i>(__vector));

		else
			return _mm512_movepi8_mask(__as<__m512i>(__vector));
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512DQ, 512, _Type_>:
	_To_bitmask<arch::ISA::AVX512F, 512, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 8)
			return _mm512_movepi64_mask(__as<__m512i>(__vector));

		else if constexpr (sizeof(_Type_) == 4)
			return _mm512_movepi32_mask(__as<__m512i>(__vector));

		else 
			return _To_bitmask<arch::ISA::AVX512F, 512, _Type_>()(__vector);
	}
};

template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512BWDQ, 512, _Type_>: 
	_To_bitmask<arch::ISA::AVX512BW, 512, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_Type_) >= 4)
			return _To_bitmask<arch::ISA::AVX512DQ, 512, _Type_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512BW, 512, _Type_>()(__vector);
	}
};

template <class _Type_> 
struct _To_bitmask<arch::ISA::AVX512VLBW, 256, _Type_>:
	_To_bitmask<arch::ISA::AVX512VLF, 256, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 4 || sizeof(_Type_) == 8)
			return _To_bitmask<arch::ISA::AVX512VLF, 256, _Type_>()(__vector);

		else if constexpr (sizeof(_Type_) == 2)
			return _mm256_movepi16_mask(__as<__m256i>(__vector));

		else
			return _mm256_movepi8_mask(__as<__m256i>(__vector));
	}
};


template <class _Type_> 
struct _To_bitmask<arch::ISA::AVX512VLDQ, 256, _Type_>: 
	_To_bitmask<arch::ISA::AVX512VLF, 256, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 8)
			return _mm256_movepi64_mask(__as<__m256i>(__vector));

		else if constexpr (sizeof(_Type_) == 4)
			return _mm256_movepi32_mask(__as<__m256i>(__vector));

		else
			return _To_bitmask<arch::ISA::AVX512VLF, 256, _Type_>()(__vector);
	}
};


template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512VLBWDQ, 256, _Type_> :
	_To_bitmask<arch::ISA::AVX512VLBW, 256, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_Type_) >= 4)
			return _To_bitmask<arch::ISA::AVX512VLDQ, 256, _Type_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512VLBW, 256, _Type_>()(__vector);
	}
};

template <class _Type_> 
struct _To_bitmask<arch::ISA::AVX512VLBW, 128, _Type_>:
	_To_bitmask<arch::ISA::AVX512VLF, 128, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 4 || sizeof(_Type_) == 8)
			return _To_bitmask<arch::ISA::AVX512VLF, 128, _Type_>()(__vector);

		else if constexpr (sizeof(_Type_) == 2)
			return _mm_movepi16_mask(__as<__m128i>(__vector));

		else
			return _mm_movepi8_mask(__as<__m128i>(__vector));
	}
};

template <class _Type_> 
struct _To_bitmask<arch::ISA::AVX512VLDQ, 128, _Type_>: 
	_To_bitmask<arch::ISA::AVX512VLF, 128, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 8)
			return _mm_movepi64_mask(__as<__m128i>(__vector));

		else if constexpr (sizeof(_Type_) == 4)
			return _mm_movepi32_mask(__as<__m128i>(__vector));

		else
			return _To_bitmask<arch::ISA::AVX512VLF, 128, _Type_>()(__vector);
	}
};


template <class _Type_>
struct _To_bitmask<arch::ISA::AVX512VLBWDQ, 128, _Type_>:
	_To_bitmask<arch::ISA::AVX512VLBW, 128, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (sizeof(_Type_) >= 4)
			return _To_bitmask<arch::ISA::AVX512VLDQ, 128, _Type_>()(__vector);
		else
			return _To_bitmask<arch::ISA::AVX512VLBW, 128, _Type_>()(__vector);
	}
};

template <class _Type_> struct _To_bitmask<arch::ISA::SSE3, 128, _Type_> : _To_bitmask<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::SSSE3, 128, _Type_> : _To_bitmask<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::SSE41, 128, _Type_> : _To_bitmask<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::SSE42, 128, _Type_> : _To_bitmask<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX, 128, _Type_> : _To_bitmask<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX2, 128, _Type_> : _To_bitmask<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::FMA3, 128, _Type_> : _To_bitmask<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX2FMA3, 128, _Type_> : _To_bitmask<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI, 512, _Type_> : _To_bitmask<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2, 512, _Type_> : _To_bitmask<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _To_bitmask<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _To_bitmask<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _To_bitmask<arch::ISA::FMA3, 256, _Type_> : _To_bitmask<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX2FMA3, 256, _Type_> : _To_bitmask<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VLF, 256, _Type_> : _To_bitmask<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMIVL, 256, _Type_> : _To_bitmask<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _To_bitmask<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _To_bitmask<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VLF, 128, _Type_> : _To_bitmask<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMIVL, 128, _Type_> : _To_bitmask<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _To_bitmask<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _To_bitmask<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _To_bitmask<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _To_bitmask<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
