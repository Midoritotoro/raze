#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/arithmetic/Divisor.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	uint32			_Width_,
	arithmetic_type	_Type_,
	intrin_type		_IntrinType_>
raze_nodiscard raze_always_inline _IntrinType_ __div_fallback(
	_IntrinType_ __left,
	_IntrinType_ __right) noexcept
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
	
	alignas(sizeof(_IntrinType_)) _Type_ __destination[__length];

	alignas(sizeof(_IntrinType_)) _Type_ __left_array[__length];
	alignas(sizeof(_IntrinType_)) _Type_ __right_array[__length];

	_Store<_ISA_>()(__left_array, __left, __aligned_policy{});
	_Store<_ISA_>()(__right_array, __right, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__destination[__i] = __left_array[__i] / __right_array[__i];

	return _Load<_ISA_, _IntrinType_>()(__destination, __aligned_policy{});
}

template <
	arch::ISA		_ISA_,
	uint32			_Width_,
	arithmetic_type	_Type_,
	intrin_type		_IntrinType_>
raze_nodiscard raze_always_inline _IntrinType_ __div_fallback(
	_IntrinType_	__left,
	_Type_			__right) noexcept
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

	alignas(sizeof(_IntrinType_)) _Type_ __destination[__length];
	alignas(sizeof(_IntrinType_)) _Type_ __left_array[__length];

	_Store<_ISA_>()(__left_array, __left, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__destination[__i] = __left_array[__i] / __right;

	return _Load<_ISA_, _IntrinType_>()(__destination, __aligned_policy{});
}


template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Div {
	template <intrin_type _IntrinType_>
	raze_nodiscard raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right) const noexcept
	{

	}
};

__RAZE_VX_NAMESPACE_END
