#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>

#include <src/raze/datapar/arithmetic/VerticalMax.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>

#include <src/raze/datapar/reduce/Fold.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA   _ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_vertical_max_wrapper {
	template <class _IntrinType_>
	raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Simd_vertical_max<_ISA_, _Width_, _DesiredType_>()(__left, __right);
	}
};

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_horizontal_max;

template <class _DesiredType_>
struct _Simd_horizontal_max<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Simd_fold<arch::ISA::SSE2, 128, _DesiredType_>()(__vector, 
			_Simd_vertical_max_wrapper<arch::ISA::SSE2, 128, _DesiredType_>{});
	}
};

template <class _DesiredType_> 
struct _Simd_horizontal_max<arch::ISA::SSSE3, 128, _DesiredType_>:
	_Simd_horizontal_max<arch::ISA::SSE3, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Simd_fold<arch::ISA::SSSE3, 128, _DesiredType_>()(__vector,
			_Simd_vertical_max_wrapper<arch::ISA::SSSE3, 128, _DesiredType_>{});
	}
};

template <class _DesiredType_>
struct _Simd_horizontal_max<arch::ISA::SSE41, 128, _DesiredType_> :
	_Simd_horizontal_max<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Simd_fold<arch::ISA::SSE41, 128, _DesiredType_>()(__vector,
			_Simd_vertical_max_wrapper<arch::ISA::SSE41, 128, _DesiredType_>{});
	}
};

template <class _DesiredType_>
struct _Simd_horizontal_max<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		return _Simd_fold<arch::ISA::AVX2, 256, _DesiredType_>()(__vector,
			_Simd_vertical_max_wrapper<arch::ISA::AVX2, 256, _DesiredType_>{});
	}
};

template <class _DesiredType_>
struct _Simd_horizontal_max<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Simd_fold<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector,
			_Simd_vertical_max_wrapper<arch::ISA::AVX512F, 512, _DesiredType_>{});
	}
};

template <class _DesiredType_>
struct _Simd_horizontal_max<arch::ISA::AVX512BW, 512, _DesiredType_>:
	_Simd_horizontal_max<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_DesiredType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Simd_fold<arch::ISA::AVX512BW, 512, _DesiredType_>()(__vector, 
			_Simd_vertical_max_wrapper<arch::ISA::AVX512BW, 512, _DesiredType_>{});
	}
};

template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::SSE3, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::SSE42, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX2, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512DQ, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLF, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLBW, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMIVL, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLF, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLBW, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMIVL, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_horizontal_max<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_>: _Simd_horizontal_max<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
