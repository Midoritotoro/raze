#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/memory/AlignmentPolicy.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_IntrinType_>
struct _Simd_load;

template <class _IntrinType_>
struct _Simd_load<arch::ISA::SSE2, 128, _IntrinType_> {
	template <class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_AlignmentPolicy_ && = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m128i>)
				return _mm_load_si128(reinterpret_cast<const __m128i*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m128d>)
				return _mm_load_pd(reinterpret_cast<const double*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m128>)
				return _mm_load_ps(reinterpret_cast<const float*>(__address));
		}
		else {
			if      constexpr (std::is_same_v<_IntrinType_, __m128i>)
				return _mm_loadu_si128(reinterpret_cast<const __m128i*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m128d>)
				return _mm_loadu_pd(reinterpret_cast<const double*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m128>)
				return _mm_loadu_ps(reinterpret_cast<const float*>(__address));
		}
	}
};

template <class _IntrinType_> 
struct _Simd_load<arch::ISA::SSE3, 128, _IntrinType_>:
	_Simd_load<arch::ISA::SSE2, 128, _IntrinType_>
{
	template <class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment == false)
			return __intrin_bitcast<_IntrinType_>(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(__address)));
		else
			return _Simd_load<arch::ISA::SSE2, 128, _IntrinType_>()(__address, __alignment_policy);
	}
};


template <class _IntrinType_>
struct _Simd_load<arch::ISA::AVX2, 256, _IntrinType_> {
	template <class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m256i>)
				return _mm256_load_si256(reinterpret_cast<const __m256i*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m256d>)
				return _mm256_load_pd(reinterpret_cast<const double*>(__address));

			else if constexpr (std::is_same_v<_IntrinType_, __m256>)
				return _mm256_load_ps(reinterpret_cast<const float*>(__address));
		}
		else {
			return __intrin_bitcast<_IntrinType_>(_mm256_lddqu_si256(reinterpret_cast<const __m256i*>(__address)));
		}
	}
};

template <class _IntrinType_>
struct _Simd_load<arch::ISA::AVX512F, 512, _IntrinType_> {
	template <class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m512i>)
				return _mm512_load_si512(__address);

			else if constexpr (std::is_same_v<_IntrinType_, __m512d>)
				return _mm512_load_pd(__address);

			else if constexpr (std::is_same_v<_IntrinType_, __m512>)
				return _mm512_load_ps(__address);
		}
		else {
			if      constexpr (std::is_same_v<_IntrinType_, __m512i>)
				return _mm512_loadu_si512(__address);

			else if constexpr (std::is_same_v<_IntrinType_, __m512d>)
				return _mm512_loadu_pd(__address);

			else if constexpr (std::is_same_v<_IntrinType_, __m512>)
				return _mm512_loadu_ps(__address);
		}
	}
};

template <class _IntrinType_> struct _Simd_load<arch::ISA::SSSE3, 128, _IntrinType_> : _Simd_load<arch::ISA::SSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::SSE41, 128, _IntrinType_> : _Simd_load<arch::ISA::SSSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::SSE42, 128, _IntrinType_> : _Simd_load<arch::ISA::SSE41, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX2, 128, _IntrinType_> : _Simd_load<arch::ISA::SSE42, 128, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512BW, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512DQ, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512BWDQ, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMI, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512BWDQ, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLF, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX2, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLDQ, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLF, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLDQ, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_load<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_> : _Simd_load<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> {};

__RAZE_DATAPAR_NAMESPACE_END
