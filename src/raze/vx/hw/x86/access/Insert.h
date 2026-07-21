#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>

#include <raze/math/BitCast.h>
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

template <arch::ISA _ISA_>
struct _Insert_vector {
	template <intrin_type _Tp_, sizetype _I_, intrin_type _Type_>
	raze_static_operator raze_always_inline void operator()(_Tp_& __vector, std::integral_constant<sizetype, _I_> __index, _Type_ __value) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == sizeof(_Type_)) __vector = __as<_Tp_>(__value);
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (sizeof(_Tp_) == 16) __vector = _mm256_inserti128_si256(__as<__m256i>(__vector), __as<__m128i>(__value), __index);
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (sizeof(_Tp_) == 16) __vector = _mm512_inserti32x4(__as<__m512i>(__vector), __as<__m128i>(__value), __index);
			else if constexpr (sizeof(_Tp_) == 32) __vector = _mm512_inserti32x8(__as<__m512i>(__vector), __as<__m256i>(__value), __index);
		}
	}
};

__RAZE_VX_NAMESPACE_END
