#pragma once 

#include <src/raze/vx/hw/x86/mask/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _To_bitmask {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_integral_v<_IntrinType_>)
			return __vector;

		else if constexpr (sizeof(_Type_) == 2)
			return static_cast<uint16>(_mm_movemask_epi8(__as<__m128i>(__vector)));
		
		else
			return _To_mask<arch::ISA::SSE2, 128, _Type_>()(__vector);
	}
};

__RAZE_VX_NAMESPACE_END
