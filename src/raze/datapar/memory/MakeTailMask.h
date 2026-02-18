#pragma once 

#include <src/raze/datapar/bitwise/MaskConvert.h>
#include <array>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_make_tail_mask;

template <class _DesiredType_>
struct _Simd_make_tail_mask<arch::ISA::SSE2, 128, _DesiredType_> {
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(uint32 __bytes) raze_const_operator noexcept
	{
		alignas(16) static constexpr uint32 __tail_mask[8] = { ~0u, ~0u, ~0u, ~0u, 0, 0, 0, 0 };
		return _mm_load_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8*>(__tail_mask) + (16 - __bytes)));
	}
};


template <class _DesiredType_>
struct _Simd_make_tail_mask<arch::ISA::AVX2, 256, _DesiredType_> {
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(uint32 __bytes) raze_const_operator noexcept
	{
		alignas(32) static constexpr uint32 __tail_mask[16] = { ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 0, 0, 0, 0, 0, 0, 0, 0 };
		return _mm256_load_si256(reinterpret_cast<const __m256i*>(reinterpret_cast<const uint8*>(__tail_mask) + (32 - __bytes)));
	}
};

template <class _DesiredType_>
struct _Simd_make_tail_mask<arch::ISA::AVX512F, 512, _DesiredType_> {
private:
	template <
		sizetype    _Size_,
		class       _DesiredType_>
	static constexpr auto __make_tail_mask_table() {
		using _MaskType = typename IntegerForSize<((_Size_ / sizeof(_DesiredType_)) / 64)>::Unsigned;
		std::array<_MaskType, 64> __table{};

		for (auto __bytes = 0; __bytes < 64; ++__bytes)
			__table[__bytes] = (__bytes == 0) ? 0 : (static_cast<_MaskType>((_MaskType(1) << (__bytes / sizeof(_DesiredType_))) - 1));

		return __table;
	}
public:
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(uint32 __bytes) raze_const_operator noexcept
	{
		alignas(64) static constexpr auto __tail_table = __make_tail_mask_table<512, _DesiredType_>();
		return __tail_table[__bytes];
	}
};

template <class _DesiredType_>
struct _Simd_make_tail_mask<arch::ISA::AVX512VLF, 256, _DesiredType_>:
	_Simd_make_tail_mask<arch::ISA::AVX2, 256, _DesiredType_> 
{
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(uint32 __bytes) raze_const_operator noexcept
	{
		using _MaskType = type_traits::__deduce_simd_mask_type<arch::ISA::AVX512VLF, _DesiredType_, 256>;
		return (__bytes == 0) ? 0 : (static_cast<_MaskType>((_MaskType(1) << (__bytes / sizeof(_DesiredType_))) - 1));
	}
};

template <class _DesiredType_>
struct _Simd_make_tail_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Simd_make_tail_mask<arch::ISA::AVX2, 128, _DesiredType_>
{
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(uint32 __bytes) raze_const_operator noexcept
	{
		using _MaskType = type_traits::__deduce_simd_mask_type<arch::ISA::AVX512VLF, _DesiredType_, 128>;
		return (__bytes == 0) ? 0 : (static_cast<_MaskType>((_MaskType(1) << (__bytes / sizeof(_DesiredType_))) - 1));
	}
};

template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512BW, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_make_tail_mask<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_make_tail_mask<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
