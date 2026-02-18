#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/memory/AlignmentPolicy.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Simd_store;

template <>
struct _Simd_store<arch::ISA::SSE2, 128> {
	template <
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m128i>)
				return _mm_store_si128(reinterpret_cast<__m128i*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m128d>)
				return _mm_store_pd(reinterpret_cast<double*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m128>)
				return _mm_store_ps(reinterpret_cast<float*>(__address), __vector);
		}
		else {
			if      constexpr (std::is_same_v<_IntrinType_, __m128i>)
				return _mm_storeu_si128(reinterpret_cast<__m128i*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m128d>)
				return _mm_storeu_pd(reinterpret_cast<double*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m128>)
				return _mm_storeu_ps(reinterpret_cast<float*>(__address), __vector);
		}
	}
};

template <>
struct _Simd_store<arch::ISA::AVX2, 256> {
	template <
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m256i>)
				return _mm256_store_si256(reinterpret_cast<__m256i*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m256d>)
				return _mm256_store_pd(reinterpret_cast<double*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m256>)
				return _mm256_store_ps(reinterpret_cast<float*>(__address), __vector);
		}
		else {
			if      constexpr (std::is_same_v<_IntrinType_, __m256i>)
				return _mm256_storeu_si256(reinterpret_cast<__m256i*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m256d>)
				return _mm256_storeu_pd(reinterpret_cast<double*>(__address), __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m256>)
				return _mm256_storeu_ps(reinterpret_cast<float*>(__address), __vector);
		}
	}
};

template <>
struct _Simd_store<arch::ISA::AVX512F, 512> {
	template <
		class _IntrinType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if      constexpr (std::is_same_v<_IntrinType_, __m512i>)
				return _mm512_store_si512(__address, __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m512d>)
				return _mm512_store_pd(__address, __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m512>)
				return _mm512_store_ps(__address, __vector);
		}
		else {
			if      constexpr (std::is_same_v<_IntrinType_, __m512i>)
				return _mm512_storeu_si512(__address, __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m512d>)
				return _mm512_storeu_pd(__address, __vector);

			else if constexpr (std::is_same_v<_IntrinType_, __m512>)
				return _mm512_storeu_ps(__address, __vector);
		}
	}
};

template <> struct _Simd_store<arch::ISA::SSE3, 128> : _Simd_store<arch::ISA::SSE2, 128> {};
template <> struct _Simd_store<arch::ISA::SSSE3, 128> : _Simd_store<arch::ISA::SSE3, 128> {};
template <> struct _Simd_store<arch::ISA::SSE41, 128> : _Simd_store<arch::ISA::SSSE3, 128> {};
template <> struct _Simd_store<arch::ISA::SSE42, 128> : _Simd_store<arch::ISA::SSE41, 128> {};
template <> struct _Simd_store<arch::ISA::AVX2, 128> : _Simd_store<arch::ISA::SSE42, 128> {};

template <> struct _Simd_store<arch::ISA::AVX512BW, 512> : _Simd_store<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512DQ, 512> : _Simd_store<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512BWDQ, 512> : _Simd_store<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI, 512> : _Simd_store<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2, 512> : _Simd_store<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMIDQ, 512> : _Simd_store<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2DQ, 512> : _Simd_store<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Simd_store<arch::ISA::AVX512VLF, 256> : _Simd_store<arch::ISA::AVX2, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VLBW, 256> : _Simd_store<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VLDQ, 256> : _Simd_store<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VLBWDQ, 256> : _Simd_store<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMIVL, 256> : _Simd_store<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2VL, 256> : _Simd_store<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMIVLDQ, 256> : _Simd_store<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2VLDQ, 256> : _Simd_store<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Simd_store<arch::ISA::AVX512VLF, 128> : _Simd_store<arch::ISA::AVX2, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VLBW, 128> : _Simd_store<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VLDQ, 128> : _Simd_store<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VLBWDQ, 128> : _Simd_store<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMIVL, 128> : _Simd_store<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2VL, 128> : _Simd_store<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMIVLDQ, 128> : _Simd_store<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Simd_store<arch::ISA::AVX512VBMI2VLDQ, 128> : _Simd_store<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
