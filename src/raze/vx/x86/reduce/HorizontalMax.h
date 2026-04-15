#pragma once 

#include <src/raze/vx/arithmetic/VerticalMax.h>
#include <src/raze/vx/reduce/Fold.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA   _ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Vertical_max_wrapper {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		return _Vertical_max<_ISA_, _Width_, _Type_>()(__left, __right);
	}
};

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Horizontal_max {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _Type_ 
		operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _Fold<_ISA_, _Width_, _Type_>()(__vector,
			_Vertical_max_wrapper<_ISA_, _Width_, _Type_>{});
	}
};
__RAZE_VX_NAMESPACE_END
