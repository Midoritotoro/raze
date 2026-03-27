#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Rotate_right;

template <>
struct _Rotate_right<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{

	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};

template <> struct _Rotate_right<arch::ISA::SSE3, 128> : _Rotate_right<arch::ISA::SSE2, 128> {};

template <>
struct _Rotate_right<arch::ISA::SSSE3, 128> :
	_Rotate_right<arch::ISA::SSE3, 128>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		return _Rotate_right<arch::ISA::SSE2, 128>()(__left, __shift);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};

template <>
struct _Rotate_right<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{

	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};

template <>
struct _Rotate_right<arch::ISA::AVX512VLF, 256> :
	_Rotate_right<arch::ISA::AVX2, 256>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		return _Rotate_right<arch::ISA::AVX2, 256>()(__left, __shift);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};


template <>
struct _Rotate_right<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{

	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};

template <>
struct _Rotate_right<arch::ISA::AVX512BW, 512> :
	_Rotate_right<arch::ISA::AVX512F, 512>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{

	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{

	}
};


template <> struct _Rotate_right<arch::ISA::SSE41, 128> : _Rotate_right<arch::ISA::SSSE3, 128> {};
template <> struct _Rotate_right<arch::ISA::SSE42, 128> : _Rotate_right<arch::ISA::SSE41, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX2, 128> : _Rotate_right<arch::ISA::SSE42, 128> {};

template <> struct _Rotate_right<arch::ISA::AVX512DQ, 512> : _Rotate_right<arch::ISA::AVX512F, 512> {};
template <> struct _Rotate_right<arch::ISA::AVX512BWDQ, 512> : _Rotate_right<arch::ISA::AVX512BW, 512> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI, 512> : _Rotate_right<arch::ISA::AVX512BW, 512> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2, 512> : _Rotate_right<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMIDQ, 512> : _Rotate_right<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2DQ, 512> : _Rotate_right<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Rotate_right<arch::ISA::AVX512VLBW, 256> : _Rotate_right<arch::ISA::AVX512VLF, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VLDQ, 256> : _Rotate_right<arch::ISA::AVX512VLF, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VLBWDQ, 256> : _Rotate_right<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMIVL, 256> : _Rotate_right<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2VL, 256> : _Rotate_right<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 256> : _Rotate_right<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2VLDQ, 256> : _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Rotate_right<arch::ISA::AVX512VLF, 128> : _Rotate_right<arch::ISA::AVX2, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VLBW, 128> : _Rotate_right<arch::ISA::AVX512VLF, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VLDQ, 128> : _Rotate_right<arch::ISA::AVX512VLF, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VLBWDQ, 128> : _Rotate_right<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMIVL, 128> : _Rotate_right<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2VL, 128> : _Rotate_right<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 128> : _Rotate_right<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Rotate_right<arch::ISA::AVX512VBMI2VLDQ, 128> : _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
