#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_,
	class		_IntrinType_,
	class		_IndexIntrinType_>
raze_nodiscard raze_always_inline _IntrinType_ __shuffle_fallback(
	_IntrinType_		__vector,
	_IndexIntrinType_	__indices) noexcept 
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
	using _IndexType = typename IntegerForSizeof<_DesiredType_>::Unsigned;

	alignas(sizeof(_IntrinType_)) _IndexType __indices_array[__length];

	alignas(sizeof(_IntrinType_)) _DesiredType_ __values_array[__length];
	alignas(sizeof(_IntrinType_)) _DesiredType_ __result_array[__length];

	_Store<_ISA_, _Width_>()(__indices_array, __indices, __aligned_policy{});
	_Store<_ISA_, _Width_>()(__values_array, __vector, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__result_array[__i] = __values_array[__indices_array[__i]];

	return _Load<_ISA_, _Width_, _IntrinType_>()(__result_array, __aligned_policy{});
}

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_,
	class		_IntrinType_,
	uint64 ...	_Indices_>
raze_nodiscard raze_always_inline _IntrinType_ __shuffle_fallback(
	_IntrinType_ __vector,
	std::integer_sequence<uint64, _Indices_...>) noexcept
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
	using _IndexType = typename IntegerForSizeof<_DesiredType_>::Unsigned;

	alignas(sizeof(_IntrinType_)) constexpr _IndexType __indices_array[] = { _Indices_... };

	alignas(sizeof(_IntrinType_)) _DesiredType_ __values_array[__length];
	alignas(sizeof(_IntrinType_)) _DesiredType_ __result_array[__length];

	_Store<_ISA_, _Width_>()(__values_array, __vector, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__result_array[__i] = __values_array[__indices_array[__i]];

	return _Load<_ISA_, _Width_, _IntrinType_>()(__result_array, __aligned_policy{});
}

template <
	class	_Type_, 
	uint64	_Length_,
	uint64	_Alignment_>
struct _Aligned_constexpr_array {
	alignas(_Alignment_) _Type_ __array[_Length_];
};

template <
	uint64		_Alignment_, 
	uint64 ...	_Indices_>
constexpr auto __expand_16bit_shuffle_to_pshufb_indices(std::integer_sequence<uint64, _Indices_...>) noexcept {
	constexpr auto __length = sizeof...(_Indices_);
	constexpr uint64 __indices[] = { _Indices_... };

	_Aligned_constexpr_array<uint8, __length << 1, _Alignment_> __result {};

	for (auto __i = 0; __i < __length; ++__i) {
		__result.__array[__i << 1] = static_cast<uint8>(__indices[__i] << 1);
		__result.__array[(__i << 1) + 1] = static_cast<uint8>((__indices[__i] << 1) + 1);
	}

	return __result;
}

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Shuffle;

template <class	_DesiredType_>
struct _Shuffle<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _IndexIntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_		__vector,
		_IndexIntrinType_	__indices) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) == 8) {
			const auto __first_index = _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__indices));
			const auto __second_index = _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(_mm_shuffle_pd(
				__intrin_bitcast<__m128d>(__indices), __intrin_bitcast<__m128d>(__indices), 0x01)));

			if (__first_index == 0) {
				if (__second_index == 0)
					return __intrin_bitcast<_IntrinType_>(_mm_unpacklo_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector)));
				else
					return __vector;
			}
			else {
				if (__second_index == 0)
					return __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 0x01));
				else
					return __intrin_bitcast<_IntrinType_>(_mm_unpackhi_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector)));
			}
		}
		else {
			return __shuffle_fallback<arch::ISA::SSE2, 128, _DesiredType_>(__vector, __indices);
		}
	}

	template <
		class		_IntrinType_,
		uint64 ...	_Indices_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_								__vector,
		std::integer_sequence<uint64, _Indices_...> __indices_sequence) raze_const_operator noexcept
	{
		constexpr uint64 __indices[] = { _Indices_... };

		if constexpr (sizeof(_DesiredType_) == 8) {
			if constexpr (__indices[0] == 0) {
				if constexpr (__indices[1] == 0)
					return __intrin_bitcast<_IntrinType_>(_mm_unpacklo_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector)));
				else
					return __vector;
			}
			else {
				if constexpr (__indices[1] == 0)
					return __intrin_bitcast<_IntrinType_>(_mm_shuffle_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector), 0x01));
				else
					return __intrin_bitcast<_IntrinType_>(_mm_unpackhi_pd(
						__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector)));
			}
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			constexpr auto __pshufd_mask = (__indices[0] & 0x03) | ((__indices[1] & 0x03) << 2)
				| ((__indices[2] & 0x03) << 4) | ((__indices[3] & 0x03) << 6);

			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), __pshufd_mask));
		}
		else {
			return __shuffle_fallback<arch::ISA::SSE2, 128, _DesiredType_>(__vector, __indices_sequence);
		}
	}
};

template <class	_DesiredType_> struct _Shuffle<arch::ISA::SSE3, 128, _DesiredType_> : _Shuffle<arch::ISA::SSE2, 128, _DesiredType_> {};

template <class	_DesiredType_>
struct _Shuffle<arch::ISA::SSSE3, 128, _DesiredType_> :
	_Shuffle<arch::ISA::SSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _IndexIntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_		__vector,
		_IndexIntrinType_	__indices) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) == 8) {
			return _Shuffle<arch::ISA::SSE2, 128, _DesiredType_>()(__vector, __indices);
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			const auto __select_mask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0, -1, -1, -1, -1);

			const auto __base_offset_0 = _mm_slli_epi32(__intrin_bitcast<__m128i>(__indices), 2);
			const auto __base_offset_1 = _mm_add_epi32(__base_offset_0, _mm_set1_epi32(1));
			const auto __base_offset_2 = _mm_add_epi32(__base_offset_0, _mm_set1_epi32(2));
			const auto __base_offset_3 = _mm_add_epi32(__base_offset_0, _mm_set1_epi32(3));

			const auto __bytes_offset_0 = _mm_shuffle_epi8(__base_offset_0, __select_mask);
			const auto __bytes_offset_1 = _mm_shuffle_epi8(__base_offset_1, __select_mask);
			const auto __bytes_offset_2 = _mm_shuffle_epi8(__base_offset_2, __select_mask);
			const auto __bytes_offset_3 = _mm_shuffle_epi8(__base_offset_3, __select_mask);

			const auto __interleave_01 = _mm_unpacklo_epi8(__bytes_offset_0, __bytes_offset_1);
			const auto __interleave_23 = _mm_unpacklo_epi8(__bytes_offset_2, __bytes_offset_3);

			const auto __pshufb_mask = _mm_unpackhi_epi16(__interleave_01, __interleave_23);

			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(
				__intrin_bitcast<__m128i>(__vector), __pshufb_mask));
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {
			const auto __select_low8_mask = _mm_set1_epi16(0x00FF);
			const auto __select_even_mask = _mm_set_epi8(13, 12, 9, 8, 5, 4, 1, 0, 13, 12, 9, 8, 5, 4, 1, 0);
			const auto __low_indices = _mm_and_si128(__intrin_bitcast<__m128i>(__indices), __select_low8_mask);

			const auto __doubled = _mm_slli_epi16(__low_indices, 1);
			const auto __doubled_plus_one = _mm_add_epi16(__doubled, _mm_set1_epi16(1));

			const auto __low_result = _mm_unpacklo_epi8(__doubled, __doubled_plus_one);
			const auto __high_result = _mm_unpackhi_epi8(__doubled, __doubled_plus_one);

			const auto __low_selected = _mm_shuffle_epi8(__low_result, __select_even_mask);
			const auto __high_selected = _mm_shuffle_epi8(__high_result, __select_even_mask);

			const auto __pshufb_mask = _mm_alignr_epi8(__high_selected, __low_selected, 8);

			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(
				__intrin_bitcast<__m128i>(__vector), __pshufb_mask));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(
				__intrin_bitcast<__m128i>(__vector), __intrin_bitcast<__m128i>(__indices)));
		}
		else {
			return __shuffle_fallback<arch::ISA::SSSE3, 128, _DesiredType_>(__vector, __indices);
		}
	}

	template <
		class		_IntrinType_,
		uint64 ...	_Indices_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_								__vector,
		std::integer_sequence<uint64, _Indices_...> __indices_sequence) raze_const_operator noexcept
	{
		alignas(sizeof(_IntrinType_)) constexpr uint8 __indices[] = { static_cast<uint8>(_Indices_)... };

		if constexpr (sizeof(_DesiredType_) >= 4) {
			return _Shuffle<arch::ISA::SSE2, 128, _DesiredType_>()(__vector, __indices_sequence);
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {
			constexpr auto __pshufb_indices = __expand_16bit_shuffle_to_pshufb_indices<sizeof(_IntrinType_)>(__indices_sequence);
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(__intrin_bitcast<__m128i>(__vector),
				_Load<arch::ISA::SSSE3, 128, __m128i>()(__pshufb_indices.__array, __aligned_policy{})));
		}
		else if constexpr (sizeof(_DesiredType_) == 1) {
			return __intrin_bitcast<_IntrinType_>(_mm_shuffle_epi8(__intrin_bitcast<__m128i>(__vector),
				_Load<arch::ISA::SSSE3, 128, __m128i>()(__indices, __aligned_policy{})));
		}
	}
};

template <class _DesiredType_>
struct _Shuffle<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _IndexIntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_		__vector,
		_IndexIntrinType_	__indices) raze_const_operator noexcept
	{
		/*if constexpr (sizeof(_DesiredType_) == 8) {
			const auto __select_low32_mask = _mm256_set1_epi64x(0x00000000FFFFFFFF);
			const auto __offsets = _mm256_set_epi64x(3, 2, 1, 0);

			const auto __selected = _mm256_and_si256(__intrin_bitcast<__m256i>(__vector), __select_low32_mask);

			const auto __base = _mm256_srli_epi32(__selected, 1);
			const auto __base_with_offsets = _mm256_add_epi64(__base, __offsets);
		}
		else if constexpr (sizeof(_DesiredType_) == 4) {
			return __intrin_bitcast<_IntrinType_>(_mm256_permutevar8x32_epi32(
				__intrin_bitcast<__m256i>(__vector), __intrin_bitcast<__m256i>(__indices)));
		}
		else if constexpr (sizeof(_DesiredType_) == 2) {

		}*/
	}

	template <
		class		_IntrinType_,
		uint64 ...	_Indices_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_								__vector,
		std::integer_sequence<uint64, _Indices_...> __indices_sequence) raze_const_operator noexcept
	{

	}
};

template <class _DesiredType_>
struct _Shuffle<arch::ISA::AVX512VLF, 256, _DesiredType_> :
	_Shuffle<arch::ISA::AVX2, 256, _DesiredType_>
{
	
};


template <class _DesiredType_>
struct _Shuffle<arch::ISA::AVX512F, 512, _DesiredType_> {
	
};

template <class _DesiredType_>
struct _Shuffle<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Shuffle<arch::ISA::AVX512F, 512, _DesiredType_>
{
	
};


template <class _DesiredType_> struct _Shuffle<arch::ISA::SSE41, 128, _DesiredType_> : _Shuffle<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::SSE42, 128, _DesiredType_> : _Shuffle<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX2, 128, _DesiredType_> : _Shuffle<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Shuffle<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Shuffle<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
