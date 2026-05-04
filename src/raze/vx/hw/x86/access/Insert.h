#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/vx/hw/x86/access/InsertMask.h>

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Insert {
	template <intrin_or_arithmetic_type	_Tp_, arithmetic_type _Type_>
	raze_static_operator raze_always_inline void operator()(_Tp_& __vector, u8 __index, _Type_	__value) raze_const_operator noexcept {
		if constexpr (arithmetic_type<_Tp_>) {
			__vector = __value;
		}
		else {
			constexpr auto __mask = __simd_make_insert_mask<_Tp_, typename IntegerForSizeof<_Type_>::Unsigned>();

			const auto __broadcasted = _Broadcast<_ISA_, _Tp_>()(__value);
			const auto __insert_mask = _Load<_ISA_, _Tp_>()(__mask.__array + __mask.__offset - __index);

			__vector = _Select<_ISA_, _Type_>()(__broadcasted, __vector, __insert_mask);
		}
	}
};

__RAZE_VX_NAMESPACE_END
