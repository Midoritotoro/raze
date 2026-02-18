#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/MaskExpand.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_reverse;

template <class _DesiredType_>
struct _Simd_reverse<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if constexpr (sizeof(_DesiredType_) == 8) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(
				__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 1));
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0x1B));
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {
			__vector = __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(
				__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 1));

			__vector = __intrin_bitcast<_IntrinType_>(_mm_shufflehi_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));
			return __intrin_bitcast<_IntrinType_>(_mm_shufflelo_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			__vector = __intrin_bitcast<_IntrinType_>(_mm_or_si128(
				_mm_srli_epi16(__intrin_bitcast<__m128i>(__vector), 8),
				_mm_slli_epi16(__intrin_bitcast<__m128i>(__vector), 8)));

			__vector = __intrin_bitcast<_IntrinType_>(_mm_shufflelo_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));
			__vector = __intrin_bitcast<_IntrinType_>(_mm_shufflehi_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));

			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0x4E));
		}
	}
};

template <class _DesiredType_> struct _Simd_reverse<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_reverse<arch::ISA::SSE2, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Simd_reverse<arch::ISA::SSSE3, 128, _DesiredType_>: 
	_Simd_reverse<arch::ISA::SSE3, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if constexpr (sizeof(_DesiredType_) == 8) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 1));
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0x1B));
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {
			__vector = __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 1));
			__vector = __intrin_bitcast<_IntrinType_>(_mm_shufflehi_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));

			return __intrin_bitcast<_IntrinType_>(_mm_shufflelo_epi16(__intrin_bitcast<__m128i>(__vector), 0x1B));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(__intrin_bitcast<__m128i>(__vector), _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)));
		}
	}
};


template <class _DesiredType_>
struct _Simd_reverse<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if constexpr (sizeof(_DesiredType_) == 8) {
			return __intrin_bitcast<_IntrinType_>(_mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__vector), 0x1B));
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return __intrin_bitcast<_IntrinType_>(_mm256_permutevar8x32_epi32(
				__intrin_bitcast<__m256i>(__vector), _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7)));
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {
			const auto __reverse_mask = _mm256_set_epi8(
				1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
				1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14);

			const auto __shuffled = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__vector), 0x4E);
			return __intrin_bitcast<_IntrinType_>(_mm256_shuffle_epi8(__shuffled, __reverse_mask));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			const auto __reverse_mask = _mm256_set_epi8(
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

			const auto __shuffled = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__vector), 0x4E);
			return __intrin_bitcast<_IntrinType_>(_mm256_shuffle_epi8(__shuffled, __reverse_mask));
		}
	}
};

template <class _DesiredType_>
struct _Simd_reverse<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if constexpr (sizeof(_DesiredType_) == 8) {
			return __intrin_bitcast<_IntrinType_>(_mm512_permutexvar_epi64(
				_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return __intrin_bitcast<_IntrinType_>(_mm512_permutexvar_epi32(
				_mm512_setr_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0), __intrin_bitcast<__m512i>(__vector)));
		}
		else {
			const auto __low = _Simd_reverse<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256i>(__vector));
			const auto __high = _Simd_reverse<arch::ISA::AVX2, 256, _DesiredType_>()(
				_mm512_extracti64x4_epi64(__intrin_bitcast<__m512i>(__vector), 1));

			return __intrin_bitcast<_IntrinType_>(_mm512_inserti64x4(__intrin_bitcast<__m512i>(__high), __intrin_bitcast<__m256i>(__low), 1));
		}
	}
};

template <class _DesiredType_>
struct _Simd_reverse<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Simd_reverse<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if constexpr (sizeof(_DesiredType_) == 2) {
			const auto __shuffle = _mm512_setr_epi16(
				31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
				15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

			return __intrin_bitcast<_IntrinType_>(_mm512_permutexvar_epi16(__shuffle, __intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			const auto __shuffle_bytes = _mm512_setr_epi8(
				63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
				47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
				31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
				15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

			const auto __shuffle_qwords = _mm512_setr_epi64(6, 7, 4, 5, 2, 3, 0, 1);

			const auto __shuffled1 = _mm512_shuffle_epi8(__intrin_bitcast<__m512i>(__vector), __shuffle_bytes);
			return __intrin_bitcast<_IntrinType_>(_mm512_permutexvar_epi64(__shuffle_qwords, __shuffled1));
		}
		else {
			return _Simd_reverse<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector);
		}
	}
};

template <class _DesiredType_> struct _Simd_reverse<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_reverse<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_reverse<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_reverse<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_reverse<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_reverse<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
