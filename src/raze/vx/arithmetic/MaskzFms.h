#pragma once 

#include <src/raze/vx/arithmetic/Fms.h>
#include <src/raze/vx/arithmetic/MaskzAssign.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Maskz_fms;

template <class _Type_>
struct _Maskz_fms<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _Type_>()(
			_Fms<arch::ISA::SSE2, 128, _Type_>()(__x, __y, __z), __mask);
	}
};

template <class _Type_> struct _Maskz_fms<arch::ISA::SSE3, 128, _Type_> : _Maskz_fms<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::SSSE3, 128, _Type_> : _Maskz_fms<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::SSE41, 128, _Type_> : _Maskz_fms<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::SSE42, 128, _Type_> : _Maskz_fms<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX, 128, _Type_> : _Maskz_fms<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX2, 128, _Type_> : _Maskz_fms<arch::ISA::AVX, 128, _Type_> {};


template <class _Type_> 
struct _Maskz_fms<arch::ISA::FMA3, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::FMA3, 128, _Type_>()(
			_Fms<arch::ISA::FMA3, 128, _Type_>()(__x, __y, __z), __mask);
	}
};

template <class _Type_> 
struct _Maskz_fms<arch::ISA::AVX512VLF, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Bl_Maskz_assignend<arch::ISA::AVX512VLF, 128, _Type_>()(
			_Fms<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_fmsub_pd(
				__as<__m128d>(__x), __mask,
				__as<__m128d>(__y), __as<__m128d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_fmsub_ps(
				__as<__m128>(__x), __mask,
				__as<__m128>(__y), __as<__m128>(__z)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Fms<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y, __z), __mask);
		}
	}
};

template <class _Type_> 
struct _Maskz_fms<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _Type_>()(
			_Fms<arch::ISA::AVX512VLBW, 128, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_floating_point_v<_Type_>) {
			return _Maskz_fms<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _Type_>()(
				_Fms<arch::ISA::AVX512VLBW, 128, _Type_>()(__x, __y, __z), __mask);
		}
	}
};
 

template <class _Type_> 
struct _Maskz_fms<arch::ISA::AVX, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX, 256, _Type_>()(
			_Fms<arch::ISA::AVX, 256, _Type_>()(__x, __y, __z), __mask);
	}
};

template <class _Type_>
struct _Maskz_fms<arch::ISA::FMA3, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::FMA3, 256, _Type_>()(
			_Fms<arch::ISA::FMA3, 256, _Type_>()(__x, __y, __z), __mask);
	}
};

template <class _Type_>
struct _Maskz_fms<arch::ISA::AVX2, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _Type_>()(
			_Fms<arch::ISA::AVX2, 256, _Type_>()(__x, __y, __z), __mask);
	}
};

template <class _Type_> 
struct _Maskz_fms<arch::ISA::AVX2FMA3, 256, _Type_>:
	_Maskz_fms<arch::ISA::FMA3, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		if constexpr (std::is_floating_point_v<_Type_>)
			return _Maskz_fms<arch::ISA::FMA3, 256, _Type_>()(__x, __y, __z, __mask);
		else
			return _Maskz_fms<arch::ISA::AVX2, 256, _Type_>()(__x, __y, __z, __mask);
	}
};

template <class _Type_> 
struct _Maskz_fms<arch::ISA::AVX512VLF, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _Type_>()(
			_Fms<arch::ISA::AVX512VLF, 256, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_fmsub_pd(
				__as<__m256d>(__x), __mask,
				__as<__m256d>(__y), __as<__m256d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_fmsub_ps(
				__as<__m256>(__x), __mask,
				__as<__m256>(__y), __as<__m256>(__z)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLF, 256, _Type_>()(
				_Fms<arch::ISA::AVX512VLF, 256, _Type_>()(__x, __y, __z), __mask);
		}
	}
};

template <class _Type_>
struct _Maskz_fms<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _Type_>()(
			_Fms<arch::ISA::AVX512VLBW, 256, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_floating_point_v<_Type_>) {
			return _Maskz_fms<arch::ISA::AVX512VLF, 256, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _Type_>()(
				_Fms<arch::ISA::AVX512VLBW, 256, _Type_>()(__x, __y, __z), __mask);
		}
	}
};

template <class _Type_>
struct _Maskz_fms<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512F, 512, _Type_>()(
			_Fms<arch::ISA::AVX512F, 512, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_fmsub_pd(
				__as<__m512d>(__x), __mask,
				__as<__m512d>(__y), __as<__m512d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_fmsub_ps(
				__as<__m512>(__x), __mask,
				__as<__m512>(__y), __as<__m512>(__z)));
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512F, 512, _Type_>()(
				_Fms<arch::ISA::AVX512F, 512, _Type_>()(__x, __y, __z), __mask);
		}
	}
};

template <class _Type_>
struct _Maskz_fms<arch::ISA::AVX512BW, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512BW, 512, _Type_>()(
			_Fms<arch::ISA::AVX512BW, 512, _Type_>()(__x, __y, __z), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_floating_point_v<_Type_>) {
			return _Maskz_fms<arch::ISA::AVX512F, 512, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Maskz_assign<arch::ISA::AVX512BW, 512, _Type_>()(
				_Fms<arch::ISA::AVX512BW, 512, _Type_>()(__x, __y, __z), __mask);
		}
	}
};


template <class _Type_> struct _Maskz_fms<arch::ISA::AVX2FMA3, 128, _Type_> : _Maskz_fms<arch::ISA::FMA3, 128, _Type_> {};

template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512DQ, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512BWDQ, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMIDQ, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2DQ, 512, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VLDQ, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VLBWDQ, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMIVL, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2VL, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMIVLDQ, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VLDQ, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VLBWDQ, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMIVL, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2VL, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMIVLDQ, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Maskz_fms<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_>: _Maskz_fms<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
