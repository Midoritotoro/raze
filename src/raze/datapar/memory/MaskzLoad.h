#pragma once 

#include <src/raze/datapar/memory/MaskLoad.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_,
	class		_IntrinType_>
struct _Simd_maskz_load;

template <	
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::SSE2, 128, _DesiredType_, _IntrinType_> {
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&&	__alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		return _Simd_mask_load<arch::ISA::SSE2, 128, _DesiredType_>()(__address, __mask,
			_Simd_broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::SSE3, 128, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::SSE2, 128, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		return _Simd_mask_load<arch::ISA::SSE3, 128, _DesiredType_>()(__address, __mask,
			_Simd_broadcast_zeros<arch::ISA::SSE3, 128, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_> 
struct _Simd_maskz_load<arch::ISA::SSE41, 128, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::SSSE3, 128, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		return _Simd_mask_load<arch::ISA::SSE41, 128, _DesiredType_>()(__address, __mask,
			_Simd_broadcast_zeros<arch::ISA::SSE41, 128, _IntrinType_>()(), __alignment_policy);
	}
};


template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::AVX2, 128, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::SSE42, 128, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskload_epi64(reinterpret_cast<const long long*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask)));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskload_epi32(reinterpret_cast<const int*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask)));

		else if constexpr (__is_ps_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskload_ps(reinterpret_cast<const float*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask)));

		else if constexpr (__is_pd_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskload_ps(reinterpret_cast<const float*>(__address),
				__mask_convert<arch::ISA::AVX2, 128, _DesiredType_, __m128i>(__mask)));

		else
			return _Simd_mask_load<arch::ISA::AVX2, 128, _DesiredType_>()(__address, __mask,
				_Simd_broadcast_zeros<arch::ISA::AVX2, 128, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::AVX2, 256, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskload_epi64(reinterpret_cast<const long long*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask)));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskload_epi32(reinterpret_cast<const int*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask)));

		else if constexpr (__is_ps_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskload_ps(reinterpret_cast<const float*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask)));

		else if constexpr (__is_pd_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskload_ps(reinterpret_cast<const float*>(__address),
				__mask_convert<arch::ISA::AVX2, 256, _DesiredType_, __m256i>(__mask)));

		else
			return _Simd_mask_load<arch::ISA::AVX2, 256, _DesiredType_>()(__address, __mask,
				_Simd_broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::AVX512F, 512, _DesiredType_, _IntrinType_> {
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_load_epi64(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __address));

			else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_load_epi32(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __address));

			else if constexpr (__is_pd_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_load_pd(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __address));

			else if constexpr (__is_ps_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_load_ps(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __address));
		}
		else {
			if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_epi64(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __address));

			else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_epi32(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __address));

			else if constexpr (__is_pd_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_pd(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint8>(__mask), __address));

			else if constexpr (__is_ps_v<_DesiredType_>)
				return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_ps(
					__mask_convert<arch::ISA::AVX512F, 512, _DesiredType_, uint16>(__mask), __address));
		}

		return _Simd_mask_load<arch::ISA::AVX512F, 512, _DesiredType_>()(__address, __mask, 
			_Simd_broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_> 
struct _Simd_maskz_load<arch::ISA::AVX512BW, 512, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::AVX512F, 512, _DesiredType_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_epi16(__mask_convert<arch::ISA::AVX512BW, 512, _DesiredType_, uint32>(__mask), __address));

		else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_loadu_epi8(__mask_convert<arch::ISA::AVX512BW, 512, _DesiredType_, uint64>(__mask), __address));

		else
			return _Simd_maskz_load<arch::ISA::AVX512F, 512, _DesiredType_, _IntrinType_>()(__address, __mask, __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::AVX512VLF, 256, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::AVX2, 256, _DesiredType_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Simd_maskz_load<arch::ISA::AVX2, 256, _DesiredType_, _IntrinType_>()(__address, __mask, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_load_epi64(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_load_epi32(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_load_pd(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_load_ps(static_cast<uint8>(__mask), __address));
			}
			else {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_epi64(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_epi32(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_pd(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_ps(static_cast<uint8>(__mask), __address));
			}
		}

		return _Simd_mask_load<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__address, __mask,
			_Simd_broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_> 
struct _Simd_maskz_load<arch::ISA::AVX512VLBW, 256, _DesiredType_, _IntrinType_>: 
	_Simd_maskz_load<arch::ISA::AVX512VLF, 256, _DesiredType_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Simd_maskz_load<arch::ISA::AVX512VLF, 256, _DesiredType_, _IntrinType_>()(__address, __mask, __alignment_policy);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_epi16(__mask_convert<arch::ISA::AVX512VLBW, 256, _DesiredType_, uint16>(__mask), __address));

		else if constexpr (sizeof(_DesiredType_) == 1)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_loadu_epi8(__mask_convert<arch::ISA::AVX512VLBW, 256, _DesiredType_, uint32>(__mask), __address));
	}
};

template <
	class _DesiredType_,
	class _IntrinType_>
struct _Simd_maskz_load<arch::ISA::AVX512VLF, 128, _DesiredType_, _IntrinType_>:
	_Simd_maskz_load<arch::ISA::AVX2, 128, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Simd_maskz_load<arch::ISA::AVX2, 128, _DesiredType_, _IntrinType_>()(__address, __mask, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_load_epi64(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_load_epi32(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_load_pd(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_load_ps(static_cast<uint8>(__mask), __address));
			}
			else {
				if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_epi64(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_epi32(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_pd_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_pd(static_cast<uint8>(__mask), __address));

				else if constexpr (__is_ps_v<_DesiredType_>)
					return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_ps(static_cast<uint8>(__mask), __address));
			}
		}

		return _Simd_mask_load<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__address, __mask,
			_Simd_broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_>()(), __alignment_policy);
	}
};

template <
	class _DesiredType_,
	class _IntrinType_> 
struct _Simd_maskz_load<arch::ISA::AVX512VLBW, 128, _DesiredType_, _IntrinType_>: 
	_Simd_maskz_load<arch::ISA::AVX512VLF, 128, _DesiredType_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Simd_maskz_load<arch::ISA::AVX512VLF, 128, _DesiredType_, _IntrinType_>()(__address, __mask, __alignment_policy);

		else if constexpr (sizeof(_DesiredType_) == 2)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_epi16(__mask_convert<arch::ISA::AVX512VLBW, 128, _DesiredType_, uint16>(__mask), __address));

		else if constexpr (sizeof(_DesiredType_) == 1)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_loadu_epi8(__mask_convert<arch::ISA::AVX512VLBW, 128, _DesiredType_, uint32>(__mask), __address));
	}
};

template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::SSSE3, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::SSE3, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::SSE42, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::SSE41, 128, _DesiredType_, _IntrinType_> {};

template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512DQ, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512F, 512, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512BWDQ, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512BW, 512, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512BW, 512, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMI, 512, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512BWDQ, 512, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_, _IntrinType_> {};

template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VLDQ, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLF, 256, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBW, 256, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMIVL, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBW, 256, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMIVL, 256, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_, _IntrinType_> {};


template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VLDQ, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLF, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBW, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMIVL, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBW, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMIVL, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_, _IntrinType_> {};
template <class _DesiredType_, class _IntrinType_> struct _Simd_maskz_load<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_, _IntrinType_> : _Simd_maskz_load<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_, _IntrinType_> {};

__RAZE_DATAPAR_NAMESPACE_END
