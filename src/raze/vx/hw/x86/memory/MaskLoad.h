#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Mask_load;

template <class _Type_>
struct _Mask_load<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&&	__alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE2, 128, _Type_>()(
			_Load<arch::ISA::SSE2, 128, _IntrinType_>()(__address),
			__source, __mask);
	}
};

template <class _Type_>
struct _Mask_load<arch::ISA::SSE3, 128, _Type_>:
	_Mask_load<arch::ISA::SSE2, 128, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE3, 128, _Type_>()(
			_Load<arch::ISA::SSE3, 128, _IntrinType_>()(__address),
			__source, __mask);
	}
};

template <class _Type_>
struct _Mask_load<arch::ISA::SSE41, 128, _Type_>:
	_Mask_load<arch::ISA::SSSE3, 128, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE41, 128, _Type_>()(
			_Load<arch::ISA::SSE41, 128, _IntrinType_>()(__address),
			__source, __mask);
	}
};

template <class _Type_> 
struct _Mask_load<arch::ISA::AVX, 256, _Type_> {
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX, 256, _Type_>()(
			_Load<arch::ISA::AVX, 256, _IntrinType_>()(__address),
			__source, __mask);
	}
};

template <class _Type_> struct _Mask_load<arch::ISA::FMA3, 256, _Type_> : _Mask_load<arch::ISA::AVX, 256, _Type_> {};

template <class _Type_>
struct _Mask_load<arch::ISA::AVX2, 256, _Type_>: 
	_Mask_load<arch::ISA::AVX, 256, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX2, 256, _Type_>()(
			_Load<arch::ISA::AVX2, 256, _IntrinType_>()(__address),
			__source, __mask);
	}
};

template <class _Type_>
struct _Mask_load<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_load_epi64(
					__as<__m512i>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_load_epi32(
					__as<__m512i>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_pd_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_load_pd(
					__as<__m512d>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_ps_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_load_ps(
					__as<__m512>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));
		}
		else {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_loadu_epi64(
					__as<__m512i>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_loadu_epi32(
					__as<__m512i>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_pd_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_loadu_pd(
					__as<__m512d>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_ps_v<_Type_>)
				return __as<_IntrinType_>(_mm512_mask_loadu_ps(
					__as<__m512>(__source),
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));
		}

		return _Blend<arch::ISA::AVX512F, 512, _Type_>()(
			_Load<arch::ISA::AVX512F, 512, _IntrinType_>()(__address, __alignment_policy),
			__source, __mask);
	}
};

template <class _Type_> 
struct _Mask_load<arch::ISA::AVX512BW, 512, _Type_>:
	_Mask_load<arch::ISA::AVX512F, 512, _Type_> 
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm512_mask_loadu_epi16(
				__as<__m512i>(__source),
				_To_mask<arch::ISA::AVX512BW, 512, _Type_>()(__mask), __address));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm512_mask_loadu_epi8(
				__as<__m512i>(__source),
				_To_mask<arch::ISA::AVX512BW, 512, _Type_>()(__mask), __address));

		else
			return _Mask_load<arch::ISA::AVX512F, 512, _Type_>()(
				 __address, __mask, __source, __alignment_policy);
	}
};

template <class _Type_> 
struct _Mask_load<arch::ISA::AVX512VLF, 256, _Type_>:
	_Mask_load<arch::ISA::AVX2, 256, _Type_> 
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Mask_load<arch::ISA::AVX2, 256, _Type_>()(__address, __mask, __source, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_load_epi64(
						__as<__m256i>(__source), __mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_load_epi32(
						__as<__m256i>(__source), __mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_load_pd(
						__as<__m256d>(__source), __mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_load_ps(
						__as<__m256>(__source), __mask, __address));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_loadu_epi64(
						__as<__m256i>(__source), __mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_loadu_epi32(
						__as<__m256i>(__source), __mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_loadu_pd(
						__as<__m256d>(__source), __mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm256_mask_loadu_ps(
						__as<__m256>(__source), __mask, __address));
			}
		}

		return _Blend<arch::ISA::AVX512VLF, 256, _Type_>()(
			_Load<arch::ISA::AVX512VLF, 256, _IntrinType_>()(__address, __alignment_policy),
			__source, __mask);
	}
};


template <class _Type_>
struct _Mask_load<arch::ISA::AVX512VLBW, 256, _Type_> :
	_Mask_load<arch::ISA::AVX512VLF, 256, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) >= 4)
			return _Mask_load<arch::ISA::AVX512VLF, 256, _Type_>()(
				__address, __mask, __source, __alignment_policy);

		else if constexpr (sizeof(_Type_) == 2)
			return __as<_IntrinType_>(_mm256_mask_loadu_epi16(
				__as<__m256i>(__source),
				_To_mask<arch::ISA::AVX512VLBW, 256, _Type_>()(__mask), __address));

		else if constexpr (sizeof(_Type_) == 1)
			return __as<_IntrinType_>(_mm256_mask_loadu_epi8(
				__as<__m256i>(__source),
				_To_mask<arch::ISA::AVX512VLBW, 256, _Type_>()(__mask), __address));
	}
};

template <class _Type_> 
struct _Mask_load<arch::ISA::AVX512VLF, 128, _Type_>:
	_Mask_load<arch::ISA::AVX2, 128, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Mask_load<arch::ISA::AVX2, 128, _Type_>()(
				__address, __mask, __source, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_load_epi64(
						__as<__m128i>(__source), __mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_load_epi32(
						__as<__m128i>(__source), __mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_load_pd(
						__as<__m128d>(__source), __mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_load_ps(
						__as<__m128>(__source), __mask, __address));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_loadu_epi64(
						__as<__m128i>(__source), __mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_loadu_epi32(
						__as<__m128i>(__source), __mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_loadu_pd(
						__as<__m128d>(__source), __mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm_mask_loadu_ps(
						__as<__m128>(__source), __mask, __address));
			}
		}

		return _Blend<arch::ISA::AVX512VLF, 128, _Type_>()(
			_Load<arch::ISA::AVX512VLF, 128, _IntrinType_>()(__address, __alignment_policy),
			__source, __mask);
	}
};


template <class _Type_>
struct _Mask_load<arch::ISA::AVX512VLBW, 128, _Type_> :
	_Mask_load<arch::ISA::AVX512VLF, 128, _Type_>
{
	template <
		class _MaskType_,
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_IntrinType_		__source,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) >= 4)
			return _Mask_load<arch::ISA::AVX512VLF, 128, _Type_>()(
				__address, __mask, __source, __alignment_policy);

		else if constexpr (sizeof(_Type_) == 2)
			return __as<_IntrinType_>(_mm_mask_loadu_epi16(
				__as<__m128i>(__source),
				_To_mask<arch::ISA::AVX512VLBW, 128, _Type_>()(__mask), __address));

		else if constexpr (sizeof(_Type_) == 1)
			return __as<_IntrinType_>(_mm_mask_loadu_epi8(
				__as<__m128i>(__source),
				_To_mask<arch::ISA::AVX512VLBW, 128, _Type_>()(__mask), __address));
	}
};

template <class _Type_> struct _Mask_load<arch::ISA::SSSE3, 128, _Type_> : _Mask_load<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::SSE42, 128, _Type_> : _Mask_load<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX, 128, _Type_> : _Mask_load<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::FMA3, 128, _Type_> : _Mask_load<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX2, 128, _Type_> : _Mask_load<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX2FMA3, 128, _Type_> : _Mask_load<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Mask_load<arch::ISA::AVX512DQ, 512, _Type_> : _Mask_load<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512BWDQ, 512, _Type_> : _Mask_load<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI, 512, _Type_> : _Mask_load<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2, 512, _Type_> : _Mask_load<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Mask_load<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Mask_load<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};


template <class _Type_> struct _Mask_load<arch::ISA::AVX2FMA3, 256, _Type_> : _Mask_load<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VLDQ, 256, _Type_> : _Mask_load<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Mask_load<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Mask_load<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Mask_load<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Mask_load<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Mask_load<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Mask_load<arch::ISA::AVX512VLDQ, 128, _Type_> : _Mask_load<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Mask_load<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Mask_load<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Mask_load<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Mask_load<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Mask_load<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Mask_load<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
