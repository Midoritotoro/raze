#pragma once 

#include <src/raze/vx/IntrinBitcast.h>
#include <src/raze/vx/memory/Store.h>
#include <src/raze/vx/memory/MaskStore.h>
#include <src/raze/vx/memory/Load.h>
#include <src/raze/vx/shuffle/BroadcastZeros.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Rotate_right;

template <class	_Type_>
struct _Rotate_right<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
		alignas(sizeof(_IntrinType_)) _Type_ __array[__length * 2];

		_Store<arch::ISA::SSE2, 128>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::SSE2, 128>()(__array + __length, __left, __aligned_policy{});

		return _Load<arch::ISA::SSE2, 128, _IntrinType_>()(__array + (__length - __elements), __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else if constexpr (__elements == (__length / 2)) {
			return __as<_IntrinType_>(_mm_shuffle_epi32(
				__as<__m128i>(__left), 0x4E));
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};

template <class	_Type_> struct _Rotate_right<arch::ISA::SSE3, 128, _Type_> : _Rotate_right<arch::ISA::SSE2, 128, _Type_> {};

template <class	_Type_>
struct _Rotate_right<arch::ISA::SSSE3, 128, _Type_> :
	_Rotate_right<arch::ISA::SSE3, 128, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		return _Rotate_right<arch::ISA::SSE2, 128, _Type_>()(__left, __elements);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else if constexpr (__elements == (__length / 2)) {
			return __as<_IntrinType_>(_mm_shuffle_epi32(
				__as<__m128i>(__left), 0x4E));
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};

template <class _Type_>
struct _Rotate_right<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
		alignas(sizeof(_IntrinType_)) _Type_ __array[__length * 2];

		_Store<arch::ISA::AVX, 256>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::AVX, 256>()(__array + __length, __left, __aligned_policy{});

		return _Load<arch::ISA::AVX, 256, _IntrinType_>()(__array + (__length - __elements), __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};

template <class _Type_>
struct _Rotate_right<arch::ISA::AVX512VLF, 256, _Type_> :
	_Rotate_right<arch::ISA::AVX2, 256, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		return _Rotate_right<arch::ISA::AVX2, 256, _Type_>()(__left, __elements);
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};


template <class _Type_>
struct _Rotate_right<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
		alignas(sizeof(_IntrinType_)) _Type_ __array[__length * 2];

		_Store<arch::ISA::AVX512F, 512>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::AVX512F, 512>()(__array + __length, __left, __aligned_policy{});

		return _Load<arch::ISA::AVX512F, 512, _IntrinType_>()(__array + (__length - __elements), __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};

template <class _Type_>
struct _Rotate_right<arch::ISA::AVX512BW, 512, _Type_> :
	_Rotate_right<arch::ISA::AVX512F, 512, _Type_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
		alignas(sizeof(_IntrinType_)) _Type_ __array[__length * 2];

		_Store<arch::ISA::AVX512BW, 512>()(__array, __left, __aligned_policy{});
		_Store<arch::ISA::AVX512BW, 512>()(__array + __length, __left, __aligned_policy{});

		return _Load<arch::ISA::AVX512BW, 512, _IntrinType_>()(__array + (__length - __elements), __aligned_policy{});
	}

	template <
		class	_IntrinType_,
		uint32	_Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_							__left,
		std::integral_constant<uint32, _Shift_> __elements) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

		if constexpr (__elements == 0 || __elements == __length) {
			return __left;
		}
		else {
			return (*this)(__left, __elements.value);
		}
	}
};


template <class _Type_> struct _Rotate_right<arch::ISA::SSE41, 128, _Type_> : _Rotate_right<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::SSE42, 128, _Type_> : _Rotate_right<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX, 128, _Type_> : _Rotate_right<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::FMA3, 128, _Type_> : _Rotate_right<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX2, 128, _Type_> : _Rotate_right<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX2FMA3, 128, _Type_> : _Rotate_right<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Rotate_right<arch::ISA::AVX512DQ, 512, _Type_> : _Rotate_right<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512BWDQ, 512, _Type_> : _Rotate_right<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI, 512, _Type_> : _Rotate_right<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2, 512, _Type_> : _Rotate_right<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Rotate_right<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Rotate_right<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Rotate_right<arch::ISA::FMA3, 256, _Type_> : _Rotate_right<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX2, 256, _Type_> : _Rotate_right<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX2FMA3, 256, _Type_> : _Rotate_right<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLBW, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLDQ, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLF, 128, _Type_> : _Rotate_right<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLBW, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLDQ, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Rotate_right<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Rotate_right<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
