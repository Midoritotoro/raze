#pragma once 

#include <src/raze/vx/hw/x86/compare/Greater.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	uint32			_Width_,
	arithmetic_type	_Type_>
struct _Vertical_min;

template <arithmetic_type _Type_>
struct _Vertical_min<arch::ISA::SSE2, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epi16(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epu8(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_ps(__as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_pd(__as<__m128d>(__left), __as<__m128d>(__right)));
		}
		else {
			const auto __mask = _Less<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
			return _Select<arch::ISA::SSE2, 128, _Type_>()(__left, __right, __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE2, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::SSE41, 128, _Type_>:
	_Vertical_min<arch::ISA::SSSE3, 128, _Type_> 
{
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epi32(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epu32(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epu16(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epi8(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else {
			return _Vertical_min<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::SSE41, 128, _Type_>()((*this)(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::SSE41, 128, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::AVX, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_ps(__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_pd(__as<__m256d>(__left), __as<__m256d>(__right)));
		}
		else {
			const auto __mask = _Less<arch::ISA::AVX, 256, _Type_>()(__left, __right);
			return _Select<arch::ISA::AVX, 256, _Type_>()(__left, __right, __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX, 256, _Type_>()((*this)(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Vertical_min<arch::ISA::AVX2, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epi32(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epu32(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epi16(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epu16(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epi8(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epu8(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_ps(__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_pd(__as<__m256d>(__left), __as<__m256d>(__right)));
		}
		else {
			const auto __mask = _Less<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
			return _Select<arch::ISA::AVX2, 256, _Type_>()(__left, __right, __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		return _Selectz<arch::ISA::AVX2, 256, _Type_>()((*this)(__left, __right), __mask);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		return _Select<arch::ISA::AVX2, 256, _Type_>()((*this)(__left, __right), __source, __mask);
	}
};

template <arithmetic_type _Type_>
struct _Vertical_min<arch::ISA::AVX512F, 512, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epi64(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epu64(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epi32(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epu32(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_ps(__as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_pd(__as<__m512d>(__left), __as<__m512d>(__right)));
		}
		else {
			const auto __mask = _Less<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
			return _Select<arch::ISA::AVX512F, 512, _Type_>()(__left, __right, __mask);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epi64(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epu64(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epi32(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epu32(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_ps(__mask,
				__as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_pd(__mask,
				__as<__m512d>(__left), __as<__m512d>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512F, 512, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epi64(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epu64(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epi32(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epu32(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_ps(__as<__m512>(__source),
				__mask, __as<__m512>(__left), __as<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_pd(__as<__m512>(__source),
				__mask, __as<__m512>(__left), __as<__m512>(__right)));
		}
	}
};

template <arithmetic_type _Type_>
struct _Vertical_min<arch::ISA::AVX512BW, 512, _Type_>:
	_Vertical_min<arch::ISA::AVX512F, 512, _Type_> 
{
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epi8(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epu8(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epi16(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_min_epu16(__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else {
			return _Vertical_min<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Selectz<arch::ISA::AVX512F, 512, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epi16(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epu16(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epi8(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_maskz_min_epu8(__mask,
				__as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Select<arch::ISA::AVX512BW, 512, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epi16(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epu16(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epi8(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_min_epu8(__as<__m512i>(__source),
				__mask, __as<__m512i>(__left), __as<__m512i>(__right)));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::AVX512VLF, 256, _Type_>: 
	_Vertical_min<arch::ISA::AVX2, 256, _Type_> 
{
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epi64(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_min_epu64(__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else {
			return _Vertical_min<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epi64(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epu64(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epi32(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epu32(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_ps(__mask,
				__as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_pd(__mask,
				__as<__m256d>(__left), __as<__m256d>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epi64(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epu64(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epi32(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epu32(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_ps(__as<__m256>(__source),
				__mask, __as<__m256>(__left), __as<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_pd(__as<__m256d>(__source),
				__mask, __as<__m256d>(__left), __as<__m256d>(__right)));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::AVX512VLF, 128, _Type_>: 
	_Vertical_min<arch::ISA::SSE42, 128, _Type_> 
{
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epi64(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_min_epu64(__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else {
			return _Vertical_min<arch::ISA::SSE42, 128, _Type_>()(__left, __right);
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epi64(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epu64(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epi32(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epu32(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_ps(__mask,
				__as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_pd(__mask,
				__as<__m128d>(__left), __as<__m128d>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) < 4) {
			return _Select<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __source, __mask);
		}
		else if constexpr (__is_epi64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epi64(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epu64(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epi32(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epu32(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_ps(__as<__m128>(__source),
				__mask, __as<__m128>(__left), __as<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_pd(__as<__m128d>(__source),
				__mask, __as<__m128d>(__left), __as<__m128d>(__right)));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::AVX512VLBW, 128, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Vertical_min<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epi16(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epu16(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epi8(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_maskz_min_epu8(__mask,
				__as<__m128i>(__left), __as<__m128i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Selectz<arch::ISA::AVX512VLF, 128, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epi16(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epu16(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epi8(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_min_epu8(__as<__m128i>(__source),
				__mask, __as<__m128i>(__left), __as<__m128i>(__right)));
		}
	}
};

template <arithmetic_type _Type_> 
struct _Vertical_min<arch::ISA::AVX512VLBW, 256, _Type_> {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) const noexcept
	{
		return _Vertical_min<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epi16(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epu16(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epi8(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_maskz_min_epu8(__mask,
				__as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}

	template <
		intrin_type _IntrinType_,
		raw_mask_type _MaskType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__source) const noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_> || sizeof(_Type_) >= 4) {
			return _Selectz<arch::ISA::AVX512VLF, 256, _Type_>()((*this)(__left, __right), __mask);
		}
		else if constexpr (__is_epi16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epi16(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epu16(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epi8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epi8(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
		else if constexpr (__is_epu8_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_min_epu8(__as<__m256i>(__source),
				__mask, __as<__m256i>(__left), __as<__m256i>(__right)));
		}
	}
};


template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::SSE3, 128, _Type_> : _Vertical_min<arch::ISA::SSE2, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::SSSE3, 128, _Type_> : _Vertical_min<arch::ISA::SSE3, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::SSE42, 128, _Type_> : _Vertical_min<arch::ISA::SSE41, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX, 128, _Type_> : _Vertical_min<arch::ISA::SSE42, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX2, 128, _Type_> : _Vertical_min<arch::ISA::AVX, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::FMA3, 128, _Type_> : _Vertical_min<arch::ISA::AVX, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX2FMA3, 128, _Type_> : _Vertical_min<arch::ISA::AVX2, 128, _Type_> {};

template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512DQ, 512, _Type_> : _Vertical_min<arch::ISA::AVX512F, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512BWDQ, 512, _Type_> : _Vertical_min<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI, 512, _Type_> : _Vertical_min<arch::ISA::AVX512BW, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2, 512, _Type_> : _Vertical_min<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Vertical_min<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Vertical_min<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::FMA3, 256, _Type_> : _Vertical_min<arch::ISA::AVX, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX2FMA3, 256, _Type_> : _Vertical_min<arch::ISA::AVX2, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VLDQ, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VLF, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Vertical_min<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VLDQ, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VLF, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <arithmetic_type _Type_> struct _Vertical_min<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Vertical_min<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END