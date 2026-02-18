#pragma once 

#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/bitwise/MaskConvert.h>
#include <src/raze/datapar/shuffle/Compress.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_compress_store;

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::SSE2, 128, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::SSE2, 128>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_> 
struct _Simd_compress_store<arch::ISA::SSE3, 128, _DesiredType_>:
	_Simd_compress_store<arch::ISA::SSE2, 128, _DesiredType_> {};


template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::SSSE3, 128, _DesiredType_> :
	_Simd_compress_store<arch::ISA::SSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::SSSE3, 128, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::SSSE3, 128>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::SSE41, 128, _DesiredType_> :
	_Simd_compress_store<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::SSE41, 128, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::SSE41, 128>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX2, 256, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX2, 256>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512F, 512>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Simd_compress_store<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512BW, 512, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512BW, 512>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512VLF, 256, _DesiredType_> :
	_Simd_compress_store<arch::ISA::AVX2, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512VLF, 256>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Simd_compress_store<arch::ISA::AVX2, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512VLF, 128>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512VLBW, 128, _DesiredType_> :
	_Simd_compress_store<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512VLBW, 128>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_>
struct _Simd_compress_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> :
	_Simd_compress_store<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _DesiredType_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		const auto __compressed = _Simd_compress<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__vector, __mask);
		_Simd_store<arch::ISA::AVX512VLBW, 256>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_DesiredType_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};
;
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_compress_store<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_compress_store<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
