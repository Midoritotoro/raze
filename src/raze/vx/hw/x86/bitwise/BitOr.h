#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Or;

template <class _Type_>
struct _Or<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m128d>)
			return _mm_or_pd(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m128i>)
			return _mm_or_si128(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m128>)
			return _mm_or_ps(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __mask);
	}
};

template <class _Type_> struct _Or<arch::ISA::SSE3, 128, _Type_> : _Or<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::SSSE3, 128, _Type_> : _Or<arch::ISA::SSE3, 128, _Type_> {};

template <class _Type_>
struct _Or<arch::ISA::SSE41, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::SSE41, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::SSE41, 128, _Type_>()((*this)(__left, __right), __mask);
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_or_pd(__left, __right);

		else
			return __as<_IntrinType_>(_mm256_or_ps(
				__as<__m256>(__left), __as<__m256>(__right)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()((*this)(__left, __right), __mask);
	}
}; 

template <class _Type_>
struct _Or<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256i>)
			return _mm256_or_si256(__left, __right);

		else
			return _Or<arch::ISA::AVX, 256, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__left, __right), __mask);
	}
};

template <class _Type_>
struct _Or<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m512d>)
			return _mm512_or_pd(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m512i>)
			return _mm512_or_si512(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m512>)
			return _mm512_or_ps(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512F, 512, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm512_mask_or_epi64(
				__as<__m512i>(__source), __mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm512_mask_or_epi32(
				__as<__m512i>(__source), __mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm512_maskz_or_epi64(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm512_maskz_or_epi32(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX512BW, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512BW, 512, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX512BW, 512, _Type_>()((*this)(__left, __right), __mask);
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX512VLF, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm256_mask_or_epi64(
				__as<__m256i>(__source), __mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm256_mask_or_epi32(
				__as<__m256i>(__source), __mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm256_maskz_or_epi64(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm256_maskz_or_epi32(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512VLBW, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX512VLBW, 256, _Type_>()((*this)(__left, __right), __mask);
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX512VLF, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm_mask_or_epi64(
				__as<__m128i>(__source), __mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm_mask_or_epi32(
				__as<__m128i>(__source), __mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return __as<_IntrinType_>(_mm_maskz_or_epi64(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_IntrinType_>(_mm_maskz_or_epi32(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
	}
};

template <class _Type_> 
struct _Or<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Or<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX512VLBW, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}

	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX512VLBW, 128, _Type_>()((*this)(__left, __right), __mask);
	}
};


template <class _Type_> struct _Or<arch::ISA::SSE42, 128, _Type_> : _Or<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX, 128, _Type_> : _Or<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::FMA3, 128, _Type_> : _Or<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX2, 128, _Type_> : _Or<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX2FMA3, 128, _Type_> : _Or<arch::ISA::AVX, 128, _Type_> {};

template <class _Type_> struct _Or<arch::ISA::AVX512DQ, 512, _Type_> : _Or<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512BWDQ, 512, _Type_> : _Or<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI, 512, _Type_> : _Or<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2, 512, _Type_> : _Or<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Or<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Or<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Or<arch::ISA::FMA3, 256, _Type_> : _Or<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX2FMA3, 256, _Type_> : _Or<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VLDQ, 256, _Type_> : _Or<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Or<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Or<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Or<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Or<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Or<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Or<arch::ISA::AVX512VLDQ, 128, _Type_> : _Or<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Or<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Or<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Or<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Or<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Or<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Or<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
