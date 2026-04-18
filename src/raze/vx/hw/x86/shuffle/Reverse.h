#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/shuffle/Shuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <uint64 ... _Size_>
constexpr auto __reverse_integer_sequence(std::index_sequence<_Size_...>) noexcept {
	return std::integer_sequence<uint64, (sizeof...(_Size_) - 1 - _Size_)...>{};
}

template <uint64 _Size_>
using make_reverse_integer_sequence = decltype(__reverse_integer_sequence(std::make_integer_sequence<uint64, _Size_>{}));


template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Reverse {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
		return _Shuffle<_ISA_, _Width_, _Type_>()(__vector, make_reverse_integer_sequence<__length>{});
	}
};

__RAZE_VX_NAMESPACE_END
