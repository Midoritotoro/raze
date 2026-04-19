#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Abs;

template <class _Type_>
struct _Abs<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) const noexcept 
	{
		if constexpr (std::is_unsigned_v<_Type_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			const auto __high_sign = _mm_srai_epi32(__as<__m128i>(__vector), 31);
			const auto __sign = _mm_shuffle_epi32(__high_sign, 0xF5);

			const auto __invert = _mm_xor_si128(__as<__m128i>(__vector), __sign);
			return __as<_IntrinType_>(_mm_sub_epi64(__invert, __sign));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			const auto __sign = _mm_srai_epi32(__as<__m128i>(__vector), 31);
			const auto __invert = _mm_xor_si128(__as<__m128i>(__vector), __sign);

			return __as<_IntrinType_>(_mm_sub_epi32(__invert, __sign));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			const auto __negate = _mm_sub_epi16(_mm_setzero_si128(), __as<__m128i>(__vector));
			return _mm_max_epi16(__as<__m128i>(__vector), __negate);
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			const auto __negate = _mm_sub_epi8(_mm_setzero_si128(), __as<__m128i>(__vector));
			return __as<_IntrinType_>(_mm_min_epu8(__as<__m128i>(__vector), __negate));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			const auto __mask = _mm_set1_epi32(0x7FFFFFFF);
			return __as<_IntrinType_>(_mm_and_ps(__as<__m128>(__vector), __as<__m128>(__mask)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			const auto __mask = _mm_setr_epi32(-1, 0x7FFFFFFFu, -1, 0x7FFFFFFFu);
			return __as<_IntrinType_>(_mm_and_pd(__as<__m128d>(__vector), __as<__m128d>(__mask)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> struct _Abs<arch::ISA::SSE3, 128, _Type_> : _Abs<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::SSSE3, 128, _Type_> : _Abs<arch::ISA::SSE3, 128, _Type_> {};

template <class _Type_> 
struct _Abs<arch::ISA::SSE41, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) const noexcept
	{
		return _Abs<arch::ISA::SSE2, 128, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::SSE41, 128, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::SSE41, 128, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_> 
struct _Abs<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (std::is_unsigned_v<_Type_>) {
			return __vector;
		}
		else if constexpr (__is_pd_v<_Type_>) {
			const auto __mask = _mm256_setr_epi32(0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu,
				0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu);
			return __as<_IntrinType_>(_mm256_and_pd(
				__as<__m256d>(__vector), __as<__m256d>(__mask)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			const auto __mask = _mm256_set1_epi32(0x7FFFFFFFu);
			return __as<_IntrinType_>(_mm256_and_ps(
				__as<__m256>(__vector), __as<__m256>(__mask)));
		}
		else {
			const auto __low = _Abs<arch::ISA::SSE42, 128, _Type_>()(__as<__m128i>(__vector));
			const auto __high = _Abs<arch::ISA::SSE42, 128, _Type_>()(
				_mm256_extractf128_si256(__as<__m256i>(__vector), 1));

			return __as<_IntrinType_>(_mm256_insertf128_si256(
				__as<__m256i>(__low), __high, 1));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__vector), __source, __mask);
	}
};


template <class _Type_>
struct _Abs<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept 
	{
		if constexpr (std::is_unsigned_v<_Type_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			const auto __sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), __vector);
			const auto __inverted = _mm256_xor_si256(__as<__m256i>(__vector), __sign);

			return __as<_IntrinType_>(_mm256_sub_epi64(__inverted, __sign));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_abs_epi32(__as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_abs_epi16(__as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_abs_epi8(__as<__m256i>(__vector)));
		}
		else {
			return _Abs<arch::ISA::AVX, 256, _Type_>()(__vector);
		}
	}
	
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__vector), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__vector), __source, __mask);
	}
};

template <class _Type_>
struct _Abs<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (std::is_unsigned_v<_Type_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_abs_epi64(__as<__m512i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_abs_epi32(__as<__m512i>(__vector)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_abs_ps(__as<__m512>(__vector)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_abs_pd(__as<__m512d>(__vector)));
		}
		else {
			const auto __low = __as<__m256i>(__vector);
			const auto __high = _mm512_extractf64x4_pd(__as<__m512d>(__vector), 1);

			auto __result = __as<__m512i>(_Abs<arch::ISA::AVX2, 256, _Type_>()(__low));
			__result = __as<__m512i>(_mm512_insertf64x4(__as<__m512d>(__result),
				__as<__m256d>(_Abs<arch::ISA::AVX2, 256, _Type_>()(__high)), 1));

			return __as<_IntrinType_>(__result);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__vector), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_epi64(__mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_epi32(__mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_ps(__mask, __as<__m512>(__vector)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_pd(__mask, __as<__m512d>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Select<arch::ISA::AVX512F, 512, _Type_>()((*this)(__vector), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_epi64(
				__as<__m512i>(__source), __mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_epi32(
				__as<__m512i>(__source), __mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_ps(
				__as<__m512>(__source), __mask, __as<__m512>(__vector)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_pd(
				__as<__m512d>(__source), __mask, __as<__m512d>(__vector)));
		}
	}
};

template <class _Type_>
struct _Abs<arch::ISA::AVX512BW, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (__is_epi16_v<_Type_>)
			return __as<_IntrinType_>(_mm512_abs_epi16(__as<__m512i>(__vector)));

		else if constexpr (__is_epi8_v<_Type_>)
			return __as<_IntrinType_>(_mm512_abs_epi8(__as<__m512i>(__vector)));

		else
			return _Abs<arch::ISA::AVX512F, 512, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512F, 512, _Type_>()(__vector, __mask);
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_epi8(__mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_abs_epi16(__mask, __as<__m512i>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512F, 512, _Type_>()(__vector, __source, __mask);
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_epi8(
				__as<__m512i>(__source), __mask, __as<__m512i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_abs_epi16(
				__as<__m512i>(__source), __mask, __as<__m512i>(__vector)));
		}
	}
};

template <class _Type_> 
struct _Abs<arch::ISA::AVX512VLF, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_>)
			return __as<_IntrinType_>(_mm256_abs_epi64(__as<__m256i>(__vector)));
		else
			return _Abs<arch::ISA::AVX2, 256, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__vector), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_abs_epi64(__mask, __as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_abs_epi32(__mask, __as<__m256i>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Select<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__vector), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_epi64(
				__as<__m256i>(__source), __mask, __as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_epi32(
				__as<__m256i>(__source), __mask, __as<__m256i>(__vector)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_ps(
				__as<__m256>(__source), __mask, __as<__m256>(__vector)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_pd(
				__as<__m256d>(__source), __mask, __as<__m256d>(__vector)));
		}
	}
};

template <class _Type_> 
struct _Abs<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept
	{
		return _Abs<arch::ISA::AVX512VLF, 256, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512VLF, 256, _Type_>()(__vector, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_abs_epi16(__mask, __as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_abs_epi8(__mask, __as<__m256i>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512VLF, 256, _Type_>()(__vector, __source, __mask);
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_epi8(
				__as<__m256i>(__source), __mask, __as<__m256i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_abs_epi16(
				__as<__m256i>(__source), __mask, __as<__m256i>(__vector)));
		}
	}
};

template <class _Type_>
struct _Abs<arch::ISA::AVX512VLF, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_>)
			return __as<_IntrinType_>(_mm_abs_epi64(__as<__m128i>(__vector)));
		else
			return _Abs<arch::ISA::SSE42, 128, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__vector), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_abs_epi64(__mask, __as<__m128i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_abs_epi32(__mask, __as<__m128i>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4 || std::is_unsigned_v<_Type_>) {
			return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__vector), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_epi64(
				__as<__m128i>(__source), __mask, __as<__m128i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_epi32(
				__as<__m128i>(__source), __mask, __as<__m128i>(__vector)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_ps(
				__as<__m128>(__source), __mask, __as<__m128>(__vector)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_pd(
				__as<__m128d>(__source), __mask, __as<__m128d>(__vector)));
		}
	}
};

template <class _Type_> 
struct _Abs<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) const noexcept
	{
		return _Abs<arch::ISA::AVX512VLF, 128, _Type_>()(__vector);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512VLF, 128, _Type_>()(__vector, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_abs_epi16(__mask, __as<__m128i>(__vector)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_abs_epi8(__mask, __as<__m128i>(__vector)));
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4 || std::is_unsigned_v<_Type_>) {
			return _Abs<arch::ISA::AVX512VLF, 128, _Type_>()(__vector, __source, __mask);
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_epi8(
				__as<__m128i>(__source), __mask, __as<__m128i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_abs_epi16(
				__as<__m128i>(__source), __mask, __as<__m128i>(__vector)));
		}
	}
};

template <class _Type_> struct _Abs<arch::ISA::SSE42, 128, _Type_>: _Abs<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX, 128, _Type_>: _Abs<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX2, 128, _Type_> : _Abs<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::FMA3, 128, _Type_> : _Abs<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX2FMA3, 128, _Type_> : _Abs<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Abs<arch::ISA::AVX512DQ, 512, _Type_>: _Abs<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512BWDQ, 512, _Type_>: _Abs<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI, 512, _Type_>: _Abs<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2, 512, _Type_>: _Abs<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMIDQ, 512, _Type_>: _Abs<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2DQ, 512, _Type_>: _Abs<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Abs<arch::ISA::FMA3, 256, _Type_> : _Abs<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX2FMA3, 256, _Type_> : _Abs<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VLDQ, 256, _Type_>: _Abs<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VLBWDQ, 256, _Type_>: _Abs<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMIVL, 256, _Type_>: _Abs<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2VL, 256, _Type_>: _Abs<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMIVLDQ, 256, _Type_>: _Abs<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_>: _Abs<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Abs<arch::ISA::AVX512VLDQ, 128, _Type_>: _Abs<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VLBWDQ, 128, _Type_>: _Abs<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMIVL, 128, _Type_>: _Abs<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2VL, 128, _Type_>: _Abs<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMIVLDQ, 128, _Type_>: _Abs<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Abs<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_>: _Abs<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END