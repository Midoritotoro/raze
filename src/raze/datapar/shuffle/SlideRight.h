#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Slide_right;

template <>
struct _Slide_right<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()();

		alignas(sizeof(_IntrinType_)) int8 __array[sizeof(_IntrinType_) * 2];

		_Store<arch::ISA::SSE2, 128>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::SSE2, 128>()(__array + sizeof(_IntrinType_), _mm_setzero_si128(), __aligned_policy{});

		return _Load<arch::ISA::SSE2, 128, _IntrinType_>()(__array + __shift, __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		if constexpr (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()();

		else if constexpr (__shift == 0)
			return __left;

		else
			return (*this)(__left, __shift.value);
	}
};

template <> struct _Slide_right<arch::ISA::SSE3, 128> : _Slide_right<arch::ISA::SSE2, 128> {};

template <> 
struct _Slide_right<arch::ISA::SSSE3, 128>:
	_Slide_right<arch::ISA::SSE3, 128> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::SSSE3, 128, _IntrinType_>()();

		alignas(sizeof(_IntrinType_)) int8 __array[sizeof(_IntrinType_) * 2];

		_Store<arch::ISA::SSSE3, 128>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::SSSE3, 128>()(__array + sizeof(_IntrinType_), _mm_setzero_si128(), __aligned_policy{});

		return _Load<arch::ISA::SSSE3, 128, _IntrinType_>()(__array + __shift, __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		if constexpr (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()();

		else if constexpr (__shift == 0)
			return __left;

		else
			return __intrin_bitcast<_IntrinType_>(_mm_alignr_epi8(_mm_setzero_si128(),
				__intrin_bitcast<__m128i>(__left), __shift));
	}
};


template <>
struct _Slide_right<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_>()();

		alignas(sizeof(_IntrinType_)) int8 __array[sizeof(_IntrinType_) * 2];

		_Store<arch::ISA::AVX2, 256>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::AVX2, 256>()(__array + sizeof(_IntrinType_), _mm256_setzero_si256(), __aligned_policy{});

		return _Load<arch::ISA::AVX2, 256, _IntrinType_>()(__array + __shift, __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		auto __low_part = _mm256_setzero_si256();
		auto __high_part = _mm256_setzero_si256();

		if constexpr (__shift == 0) {
			return __left;
		}
		else if constexpr (__shift < 16) {
			__low_part = _mm256_inserti128_si256(__low_part, _mm256_extracti128_si256(__intrin_bitcast<__m256i>(__left), 1), 0);
			__high_part = __intrin_bitcast<__m256i>(__left);
		}
		else if constexpr (__shift < 32) {
			__high_part = _mm256_inserti128_si256(__high_part, _mm256_extracti128_si256(__intrin_bitcast<__m256i>(__left), 1), 0);
		}
		else {
			return _Broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_>()();
		}

		if constexpr ((__shift % 16) == 0) 
			return __intrin_bitcast<_IntrinType_>(__high_part);

		return __intrin_bitcast<_IntrinType_>(_mm256_alignr_epi8(__low_part, __high_part, __shift & 0xF));
	}
};

template <> 
struct _Slide_right<arch::ISA::AVX512VLF, 256>:
	_Slide_right<arch::ISA::AVX2, 256> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		return _Slide_right<arch::ISA::AVX2, 256>()(__left, __shift);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		if constexpr (__shift == 0) {
			return __left;
		}
		else if constexpr (__shift >= sizeof(_IntrinType_)) {
			return _Broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_>()();
		}
		else if constexpr ((__shift % 4) == 0) {
			return __intrin_bitcast<_IntrinType_>(_mm256_alignr_epi32(_mm256_setzero_si256(), 
				__intrin_bitcast<__m256i>(__left), (__shift >> 2) & 7));
		}
		else {
			return _Slide_right<arch::ISA::AVX2, 256>()(__left, std::integral_constant<uint32, __shift>{});
		}
	}
};

template <>
struct _Slide_right<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if (__shift >= sizeof(_IntrinType_))
			return _Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()();

		alignas(sizeof(_IntrinType_)) int8 __array[sizeof(_IntrinType_) * 2];

		_Store<arch::ISA::AVX512F, 512>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::AVX512F, 512>()(__array + sizeof(_IntrinType_), _mm512_setzero_si512(), __aligned_policy{});

		return _Load<arch::ISA::AVX512F, 512, _IntrinType_>()(__array + __shift, __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		auto __low_part = _mm512_setzero_si512();
		auto __high_part = _mm512_setzero_si512();

		if constexpr (__shift == 0) {
			return __left;
		}
		else if constexpr (__shift < 16) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x3F, __intrin_bitcast<__m512i>(__left), 
				__intrin_bitcast<__m512i>(__left), 0x39);
			__high_part = __intrin_bitcast<__m512i>(__left);
		}
		else if constexpr (__shift < 32) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x0F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x0E);
			__high_part = _mm512_maskz_shuffle_i64x2(0x3F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x39);
		}
		else if constexpr (__shift < 48) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x03, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x03);
			__high_part = _mm512_maskz_shuffle_i64x2(0x0F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x0E);
		}
		else if constexpr (__shift < 64) {
			__low_part = _mm512_setzero_si512();
			__high_part = _mm512_maskz_shuffle_i64x2(0x03, __intrin_bitcast<__m512i>(__left), 
				__intrin_bitcast<__m512i>(__left), 0x03);
		}
		else {
			return _Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()();
		}

		if constexpr ((__shift % 4) == 0) {
			return __intrin_bitcast<_IntrinType_>(_mm512_alignr_epi32(_mm512_setzero_si512(),
				__intrin_bitcast<__m512i>(__left), ((__shift >> 2) & 15)));
		}

		const auto __low256 = _mm256_alignr_epi8(__intrin_bitcast<__m256i>(__low_part),
			__intrin_bitcast<__m256i>(__high_part), __shift & 0xF);

		const auto __high256 = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__intrin_bitcast<__m512i>(__low_part), 1),
			_mm512_extracti64x4_epi64(__intrin_bitcast<__m512i>(__high_part), 1), __shift & 0xF);

		return __intrin_bitcast<_IntrinType_>(_mm512_inserti64x4(__intrin_bitcast<__m512i>(__low256), __high256, 1));
	}
};

template <> 
struct _Slide_right<arch::ISA::AVX512BW, 512>:
	_Slide_right<arch::ISA::AVX512F, 512> 
{
		template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		return _Slide_right<arch::ISA::AVX512F, 512>()(__left, __shift);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __shift) raze_const_operator noexcept
	{
		auto __low_part = _mm512_setzero_si512();
		auto __high_part = _mm512_setzero_si512();

		if constexpr (__shift == 0) {
			return __left;
		}
		else if constexpr (__shift < 16) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x3F, __intrin_bitcast<__m512i>(__left), 
				__intrin_bitcast<__m512i>(__left), 0x39);
			__high_part = __intrin_bitcast<__m512i>(__left);
		}
		else if constexpr (__shift < 32) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x0F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x0E);
			__high_part = _mm512_maskz_shuffle_i64x2(0x3F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x39);
		}
		else if constexpr (__shift < 48) {
			__low_part = _mm512_maskz_shuffle_i64x2(0x03, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x03);
			__high_part = _mm512_maskz_shuffle_i64x2(0x0F, __intrin_bitcast<__m512i>(__left),
				__intrin_bitcast<__m512i>(__left), 0x0E);
		}
		else if constexpr (__shift < 64) {
			__low_part = _mm512_setzero_si512();
			__high_part = _mm512_maskz_shuffle_i64x2(0x03, __intrin_bitcast<__m512i>(__left), 
				__intrin_bitcast<__m512i>(__left), 0x03);
		}
		else {
			return _Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()();
		}

		if constexpr ((__shift % 4) == 0) {
			return __intrin_bitcast<_IntrinType_>(_mm512_alignr_epi32(_mm512_setzero_si512(),
				__intrin_bitcast<__m512i>(__left), ((__shift >> 2) & 15)));
		}

		return __intrin_bitcast<_IntrinType_>(_mm512_alignr_epi8(__low_part, __high_part, __shift & 0xF));
	}
};

template <> struct _Slide_right<arch::ISA::SSE41, 128> : _Slide_right<arch::ISA::SSSE3, 128> {};
template <> struct _Slide_right<arch::ISA::SSE42, 128> : _Slide_right<arch::ISA::SSE41, 128> {};
template <> struct _Slide_right<arch::ISA::AVX2, 128> : _Slide_right<arch::ISA::SSE42, 128> {};

template <> struct _Slide_right<arch::ISA::AVX512DQ, 512> : _Slide_right<arch::ISA::AVX512F, 512> {};
template <> struct _Slide_right<arch::ISA::AVX512BWDQ, 512> : _Slide_right<arch::ISA::AVX512BW, 512> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI, 512> : _Slide_right<arch::ISA::AVX512BW, 512> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2, 512> : _Slide_right<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMIDQ, 512> : _Slide_right<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2DQ, 512> : _Slide_right<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Slide_right<arch::ISA::AVX512VLBW, 256> : _Slide_right<arch::ISA::AVX512VLF, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VLDQ, 256> : _Slide_right<arch::ISA::AVX512VLF, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VLBWDQ, 256> : _Slide_right<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMIVL, 256> : _Slide_right<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2VL, 256> : _Slide_right<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMIVLDQ, 256> : _Slide_right<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2VLDQ, 256> : _Slide_right<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Slide_right<arch::ISA::AVX512VLF, 128> : _Slide_right<arch::ISA::AVX2, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VLBW, 128> : _Slide_right<arch::ISA::AVX512VLF, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VLDQ, 128> : _Slide_right<arch::ISA::AVX512VLF, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VLBWDQ, 128> : _Slide_right<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMIVL, 128> : _Slide_right<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2VL, 128> : _Slide_right<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMIVLDQ, 128> : _Slide_right<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Slide_right<arch::ISA::AVX512VBMI2VLDQ, 128> : _Slide_right<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
