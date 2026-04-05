#pragma once 

#include <src/raze/datapar/arithmetic/Negate.h>
#include <src/raze/datapar/arithmetic/MaskzAssign.h>
#include <src/raze/datapar/arithmetic/MaskSub.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Maskz_negate;

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Negate<arch::ISA::SSE2, 128, _DesiredType_>()(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Negate<arch::ISA::SSE2, 128, _DesiredType_>()(__vector), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Negate<arch::ISA::AVX2, 256, _DesiredType_>()(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Negate<arch::ISA::AVX2, 256, _DesiredType_>()(__vector), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(__changed_sign, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			const auto __sign_bit = _mm512_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000,
				0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000);

			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_xor_epi64(__mask,
				__intrin_bitcast<__m512i>(__vector), __sign_bit));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			const auto __sign_bit = _mm512_set1_epi32(0x80000000);
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_xor_epi32(__mask,
				__intrin_bitcast<__m512i>(__vector), __sign_bit));
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()(), __vector);
			return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Maskz_negate<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512BW, 512, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512BW, 512, _DesiredType_>()(__changed_sign, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4) {
			return _Maskz_negate<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector, __mask);
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512BW, 512, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512BW, 512, _IntrinType_>()(), __vector);
			return _Maskz_assign<arch::ISA::AVX512BW, 512, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512VLF, 256, _DesiredType_> :
	_Maskz_negate<arch::ISA::AVX2, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__changed_sign, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			const auto __sign_bit = _mm256_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000);
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_xor_epi64(__mask,
				__intrin_bitcast<__m256i>(__vector), __sign_bit));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			const auto __sign_bit = _mm256_set1_epi32(0x80000000);
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_xor_epi32(__mask,
				__intrin_bitcast<__m256i>(__vector), __sign_bit));
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_>()(), __vector);
			return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_> :
	_Maskz_negate<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512VLBW, 256, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__changed_sign, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4) {
			return _Maskz_negate<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector, __mask);
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512VLBW, 256, _IntrinType_>()(), __vector);
			return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Maskz_negate<arch::ISA::AVX2, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__changed_sign, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			const auto __sign_bit = _mm_setr_epi32(0, 0x80000000, 0, 0x80000000);
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_xor_epi64(__mask,
				__intrin_bitcast<__m128i>(__vector), __sign_bit));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			const auto __sign_bit = _mm_set1_epi32(0x80000000);
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_xor_epi32(__mask,
				__intrin_bitcast<__m128i>(__vector), __sign_bit));
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_>()(), __vector);
			return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_> :
	_Maskz_negate<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		const auto __changed_sign = _Sub<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_Broadcast_zeros<arch::ISA::AVX512VLBW, 128, _IntrinType_>()(), __vector);
		return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__changed_sign, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4) {
			return _Maskz_negate<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector, __mask);
		}
		else {
			const auto __changed_sign = _Sub<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
				_Broadcast_zeros<arch::ISA::AVX512VLBW, 128, _IntrinType_>()(), __vector);
			return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__changed_sign, __mask);
		}
	}
};


template <class _DesiredType_> struct _Maskz_negate<arch::ISA::SSE3, 128, _DesiredType_> : _Maskz_negate<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::SSSE3, 128, _DesiredType_> : _Maskz_negate<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::SSE41, 128, _DesiredType_> : _Maskz_negate<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::SSE42, 128, _DesiredType_> : _Maskz_negate<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX2, 128, _DesiredType_> : _Maskz_negate<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_negate<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Maskz_negate<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
