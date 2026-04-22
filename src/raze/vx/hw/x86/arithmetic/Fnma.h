#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#include <src/raze/vx/hw/x86/arithmetic/Add.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA		_ISA_,
	uint32			_Width_,
	arithmetic_type	_Type_>
struct _Fnma;

template <arithmetic_type _Type_>
struct _Fnma<arch::ISA::SSE2, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Add<arch::ISA::SSE2, 128, _Type_>()(
			_Negate<arch::ISA::SSE2, 128, _Type_>()(
				_Mul<arch::ISA::SSE2, 128, _Type_>()(__x, __y)), __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::SSE3, 128, _Type_> : _Fnma<arch::ISA::SSE2, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::SSSE3, 128, _Type_> : _Fnma<arch::ISA::SSE3, 128, _Type_> {};

template <arithmetic_type _Type_>
struct _Fnma<arch::ISA::SSE41, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Add<arch::ISA::SSE41, 128, _Type_>()(
			_Negate<arch::ISA::SSE41, 128, _Type_>()(
				_Mul<arch::ISA::SSE41, 128, _Type_>()(__x, __y)), __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE41, 128, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE41, 128, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::SSE42, 128, _Type_> : _Fnma<arch::ISA::SSE41, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX, 128, _Type_> : _Fnma<arch::ISA::SSE42, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX2, 128, _Type_> : _Fnma<arch::ISA::AVX, 128, _Type_> {};


template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::FMA3, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_fnmadd_pd(
				__as<__m128d>(__x), __as<__m128d>(__y), __as<__m128d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_fnmadd_ps(
				__as<__m128>(__x), __as<__m128>(__y), __as<__m128>(__z)));
		}
		else {
			return _Add<arch::ISA::FMA3, 128, _Type_>()(
				_Negate<arch::ISA::FMA3, 128, _Type_>()(
					_Mul<arch::ISA::FMA3, 128, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::FMA3, 128, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::FMA3, 128, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Add<arch::ISA::AVX, 256, _Type_>()(
			_Negate<arch::ISA::AVX, 256, _Type_>()(
				_Mul<arch::ISA::AVX, 256, _Type_>()(__x, __y)), __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Fnma<arch::ISA::FMA3, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_fnmadd_pd(
				__as<__m256d>(__x), __as<__m256d>(__y), __as<__m256d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_fnmadd_ps(
				__as<__m256>(__x), __as<__m256>(__y), __as<__m256>(__z)));
		}
		else {
			return _Add<arch::ISA::FMA3, 256, _Type_>()(
				_Negate<arch::ISA::FMA3, 256, _Type_>()(
					_Mul<arch::ISA::FMA3, 256, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::FMA3, 256, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::FMA3, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Fnma<arch::ISA::AVX2, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Add<arch::ISA::AVX2, 256, _Type_>()(
			_Negate<arch::ISA::AVX2, 256, _Type_>()(
				_Mul<arch::ISA::AVX2, 256, _Type_>()(__x, __y)), __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX2FMA3, 256, _Type_>:
	_Fnma<arch::ISA::FMA3, 256, _Type_> 
{
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (std::is_floating_point_v<_Type_>)
			return _Fnma<arch::ISA::FMA3, 256, _Type_>()(__x, __y, __z);
		else
			return _Fnma<arch::ISA::AVX2, 256, _Type_>()(__x, __y, __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Fnma<arch::ISA::AVX512F, 512, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_fnmadd_pd(
				__as<__m512d>(__x), __as<__m512d>(__y), __as<__m512d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_fnmadd_ps(
				__as<__m512>(__x), __as<__m512>(__y), __as<__m512>(__z)));
		}
		else {
			return _Add<arch::ISA::AVX512F, 512, _Type_>()(
				_Negate<arch::ISA::AVX512F, 512, _Type_>()(
					_Mul<arch::ISA::AVX512F, 512, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || std::is_integral_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__x, __y, __z), __mask);
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_fnmadd_pd(__mask,
				__as<__m512d>(__x), __as<__m512d>(__y), __as<__m512d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_fnmadd_ps(__mask, 
				__as<__m512>(__x), __as<__m512>(__y), __as<__m512>(__z)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512F, 512, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX512BW, 512, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512F, 512, _Type_>()(__x, __y, __z);
		}
		else {
			return _Add<arch::ISA::AVX512BW, 512, _Type_>()(
				_Negate<arch::ISA::AVX512BW, 512, _Type_>()(
					_Mul<arch::ISA::AVX512BW, 512, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512F, 512, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Selectz<arch::ISA::AVX512BW, 512, _Type_>()((*this)(__x, __y, __z), __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512BW, 512, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX512VLF, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Fnma<arch::ISA::AVX2FMA3, 256, _Type_>()(__x, __y, __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || std::is_integral_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__x, __y, __z), __mask);
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_fnmadd_pd(__mask,
				__as<__m256d>(__x), __as<__m256d>(__y), __as<__m256d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_fnmadd_ps(__mask, 
				__as<__m256>(__x), __as<__m256>(__y), __as<__m256>(__z)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512VLF, 256, _Type_>()(__x, __y, __z);
		}
		else {
			return _Add<arch::ISA::AVX512VLBW, 256, _Type_>()(
				_Negate<arch::ISA::AVX512VLBW, 256, _Type_>()(
					_Mul<arch::ISA::AVX512VLBW, 256, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512VLF, 256, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Selectz<arch::ISA::AVX512VLBW, 256, _Type_>()((*this)(__x, __y, __z), __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512VLBW, 256, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX512VLF, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		return _Fnma<arch::ISA::AVX2FMA3, 128, _Type_>()(__x, __y, __z);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || std::is_integral_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__x, __y, __z), __mask);
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_fnmadd_pd(__mask,
				__as<__m128d>(__x), __as<__m128d>(__y), __as<__m128d>(__z)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_fnmadd_ps(__mask, 
				__as<__m128>(__x), __as<__m128>(__y), __as<__m128>(__z)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Fnma<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __x,
		_IntrinType_ __y,
		_IntrinType_ __z) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y, __z);
		}
		else {
			return _Add<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Negate<arch::ISA::AVX512VLF, 128, _Type_>()(
					_Mul<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y)), __z);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (sizeof(_Type_) >= 4) {
			return _Fnma<arch::ISA::AVX512VLF, 128, _Type_>()(__x, __y, __z, __mask);
		}
		else {
			return _Selectz<arch::ISA::AVX512VLBW, 128, _Type_>()((*this)(__x, __y, __z), __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__x,
		_IntrinType_	__y,
		_IntrinType_	__z,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__x, __y, __z), __source, __mask);
	}
};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX2FMA3, 128, _Type_> : _Fnma<arch::ISA::FMA3, 128, _Type_> {};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512DQ, 512, _Type_>: _Fnma<arch::ISA::AVX512F, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512BWDQ, 512, _Type_>: _Fnma<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI, 512, _Type_>: _Fnma<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2, 512, _Type_>: _Fnma<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMIDQ, 512, _Type_>: _Fnma<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2DQ, 512, _Type_>: _Fnma<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VLDQ, 256, _Type_>: _Fnma<arch::ISA::AVX512VLF, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VLBWDQ, 256, _Type_>: _Fnma<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMIVL, 256, _Type_>: _Fnma<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2VL, 256, _Type_>: _Fnma<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMIVLDQ, 256, _Type_>: _Fnma<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_>: _Fnma<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VLDQ, 128, _Type_>: _Fnma<arch::ISA::AVX512VLF, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VLBWDQ, 128, _Type_>: _Fnma<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMIVL, 128, _Type_>: _Fnma<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2VL, 128, _Type_>: _Fnma<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMIVLDQ, 128, _Type_>: _Fnma<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Fnma<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_>: _Fnma<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END