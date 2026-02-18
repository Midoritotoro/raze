#pragma once 

#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/shuffle/Blend.h>
#include <src/raze/datapar/bitwise/MaskConvert.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_mask_store;

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&&	__alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		_Simd_store<arch::ISA::SSE2, 128>()(__address, _Simd_blend<arch::ISA::SSE2, 128, _DesiredType_>()(
			__vector, _Simd_load<arch::ISA::SSE2, 128, _IntrinType_>()(__address, __alignment_policy),
			__mask_convert<arch::ISA::SSE2, 128, _DesiredType_, _IntrinType_>(__mask)), __alignment_policy);
	}
};

template <class _DesiredType_> 
struct _Simd_mask_store<arch::ISA::SSE41, 128, _DesiredType_>:
	_Simd_mask_store<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		_Simd_store<arch::ISA::SSE41, 128>()(__address, _Simd_blend<arch::ISA::SSE41, 128, _DesiredType_>()(
			__vector, _Simd_load<arch::ISA::SSE41, 128, _IntrinType_>()(__address, __alignment_policy),
			__mask_convert<arch::ISA::SSE41, 128, _DesiredType_, _IntrinType_>(__mask)), __alignment_policy);
	}
};

template <class _DesiredType_> 
struct _Simd_mask_store<arch::ISA::AVX2, 128, _DesiredType_>: 
	_Simd_mask_store<arch::ISA::SSE42, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			_mm_maskstore_epi64(reinterpret_cast<long long*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask),
				__intrin_bitcast<__m128i>(__vector));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			_mm_maskstore_epi32(reinterpret_cast<int*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask),
				__intrin_bitcast<__m128i>(__vector));

		else if constexpr (__is_pd_v<_DesiredType_>)
			_mm_maskstore_pd(reinterpret_cast<double*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask),
				__intrin_bitcast<__m128d>(__vector));

		else if constexpr (__is_ps_v<_DesiredType_>)
			_mm_maskstore_ps(reinterpret_cast<float*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask),
				__intrin_bitcast<__m128>(__vector));

		else
			_Simd_store<arch::ISA::AVX2, 128>()(__address, _Simd_blend<arch::ISA::AVX2, 128, _DesiredType_>()(
				__vector, _Simd_load<arch::ISA::AVX2, 128, _IntrinType_>()(__address, __alignment_policy), __mask), __alignment_policy);
	}
};


template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			_mm256_maskstore_epi64(reinterpret_cast<long long*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask),
				__intrin_bitcast<__m256i>(__vector));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			_mm256_maskstore_epi32(reinterpret_cast<int*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask),
				__intrin_bitcast<__m256i>(__vector));

		else if constexpr (__is_pd_v<_DesiredType_>)
			_mm256_maskstore_pd(reinterpret_cast<double*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask),
				__intrin_bitcast<__m256d>(__vector));

		else if constexpr (__is_ps_v<_DesiredType_>)
			_mm256_maskstore_ps(reinterpret_cast<float*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask),
				__intrin_bitcast<__m256>(__vector));

		else
			_Simd_store<arch::ISA::AVX2, 256>()(__address, _Simd_blend<arch::ISA::AVX2, 256, _DesiredType_>()(
				__vector, _Simd_load<arch::ISA::AVX2, 256, _IntrinType_>()(__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
				return _mm512_mask_store_epi64(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __intrin_bitcast<__m512i>(__vector));

			else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
				return _mm512_mask_store_epi32(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m512i>(__vector));

			else if constexpr (__is_pd_v<_DesiredType_>)
				return _mm512_mask_store_pd(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __intrin_bitcast<__m512d>(__vector));

			else if constexpr (__is_ps_v<_DesiredType_>)
				return _mm512_mask_store_ps(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m512>(__vector));
		}
		else {
			if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
				return _mm512_mask_storeu_epi64(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __intrin_bitcast<__m512i>(__vector));

			else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
				return _mm512_mask_storeu_epi32(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m512i>(__vector));

			else if constexpr (__is_pd_v<_DesiredType_>)
				return _mm512_mask_storeu_pd(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __intrin_bitcast<__m512d>(__vector));

			else if constexpr (__is_ps_v<_DesiredType_>)
				return _mm512_mask_storeu_ps(__address, __mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m512>(__vector));
		}

		_Simd_store<arch::ISA::AVX512F, 512>()(__address, _Simd_blend<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector, 
			_Simd_load<arch::ISA::AVX512F, 512, _IntrinType_>()(__address, __alignment_policy),
			__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, _IntrinType_>(__mask)), __alignment_policy);
	}
};

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX512BW, 512, _DesiredType_>: 
	_Simd_mask_store<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>)
			return _mm512_mask_storeu_epi16(__address, __mask_convert<arch::ISA::AVX512BW, 512, _DesiredType_, uint32>(__mask), __intrin_bitcast<__m512i>(__vector));

		else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>)
			return _mm512_mask_storeu_epi8(__address, __mask_convert<arch::ISA::AVX512BW, 512, _DesiredType_, uint64>(__mask), __intrin_bitcast<__m512i>(__vector));

		else
			return _Simd_mask_store<arch::ISA::AVX512F, 512, _DesiredType_>()(__address, __mask, __vector, __alignment_policy);
	}
};

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX512VLF, 256, _DesiredType_>: 
	_Simd_mask_store<arch::ISA::AVX2, 256, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Simd_mask_store<arch::ISA::AVX2, 256, _DesiredType_>()(__address, __mask, __vector, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return _mm256_mask_store_epi64(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256i>(__vector));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return _mm256_mask_store_epi32(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256i>(__vector));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return _mm256_mask_store_pd(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256d>(__vector));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return _mm256_mask_store_ps(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256>(__vector));
			}
			else {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return _mm256_mask_storeu_epi64(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256i>(__vector));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return _mm256_mask_storeu_epi32(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256i>(__vector));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return _mm256_mask_storeu_pd(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256d>(__vector));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return _mm256_mask_storeu_ps(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m256>(__vector));
			}
		}

		_Simd_store<arch::ISA::AVX512VLF, 256>()(__address, _Simd_blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector,
			_Simd_load<arch::ISA::AVX512VLF, 256, _IntrinType_>()(__address, __alignment_policy),
			__mask_convert<arch::ISA::AVX512VLF, 256, _DesiredType_, _IntrinType_>(__mask)), __alignment_policy);
	}
};

template <class _DesiredType_> 
struct _Simd_mask_store<arch::ISA::AVX512VLF, 128, _DesiredType_>: 
	_Simd_mask_store<arch::ISA::AVX2, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Simd_mask_store<arch::ISA::AVX2, 256, _DesiredType_>()(__address, __mask, __vector, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return _mm_mask_store_epi64(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128i>(__vector));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return _mm_mask_store_epi32(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128i>(__vector));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return _mm_mask_store_pd(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128d>(__vector));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return _mm_mask_store_ps(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128>(__vector));
			}
			else {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return _mm_mask_storeu_epi64(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128i>(__vector));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return _mm_mask_storeu_epi32(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128i>(__vector));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return _mm_mask_storeu_pd(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128d>(__vector));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return _mm_mask_storeu_ps(__address, static_cast<uint8>(__mask), __intrin_bitcast<__m128>(__vector));
			}
		}

		_Simd_store<arch::ISA::AVX512VLF, 128>()(__address, _Simd_blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector,
			_Simd_load<arch::ISA::AVX512VLF, 128, _IntrinType_>()(__address, __alignment_policy),
			__mask_convert<arch::ISA::AVX512VLF, 128, _DesiredType_, _IntrinType_>(__mask)), __alignment_policy);
	}
};

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_Simd_mask_store<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Simd_mask_store<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__address, __mask, __vector, __alignment_policy);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm_mask_storeu_epi16(__address, __mask_convert<arch::ISA::AVX512VLBW, 128, _DesiredType_, uint8>(__mask), __intrin_bitcast<__m128i>(__vector));

		else if constexpr (sizeof(_DesiredType_) == 1)
			return _mm_mask_storeu_epi8(__address, __mask_convert<arch::ISA::AVX512VLBW, 128, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m128i>(__vector));
	}
};

template <class _DesiredType_>
struct _Simd_mask_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> :
	_Simd_mask_store<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Simd_mask_store<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__address, __mask, __vector, __alignment_policy);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return _mm256_mask_storeu_epi16(__address, __mask_convert<arch::ISA::AVX512VLBW, 256, _DesiredType_, uint16>(__mask), __intrin_bitcast<__m256i>(__vector));

		else if constexpr (sizeof(_DesiredType_) == 1)
			return _mm256_mask_storeu_epi8(__address, __mask_convert<arch::ISA::AVX512VLBW, 256, _DesiredType_, uint32>(__mask), __intrin_bitcast<__m256i>(__vector));
	}
};

template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::SSE41, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};
;
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_mask_store<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_mask_store<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
