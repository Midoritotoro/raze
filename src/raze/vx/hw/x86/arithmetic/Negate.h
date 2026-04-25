#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/math/SignBit.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Negate {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x) const noexcept {
		if constexpr (arithmetic_type<_Tp_>) {
			return -__x;
		}
		else if constexpr (std::floating_point<_Type_>) {
			using _XorType = typename IntegerForSizeof<_Type_>::Signed;
			return _Xor<_ISA_, _Type_>()(__x, _Broadcast<_ISA_, _Tp_>()(math::sign_bit<_XorType>()));
		}
		else {
			return _Sub<_ISA_, _Type_>()(_Zero<_ISA_, _Tp_>()(), __x);
		}
	}
	
	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
		if constexpr (arithmetic_type<_Tp_>) {
			return __mask ? -__x : 0;
		}
		else if constexpr (std::floating_point<_Type_>) {
			using _XorType = typename IntegerForSizeof<_Type_>::Signed;
			return _Xor<_ISA_, _Type_>()(__x, _Broadcast<_ISA_, _Tp_>()(math::sign_bit<_XorType>()), __mask);
		}
		else {
			return _Sub<_ISA_, _Type_>()(_Zero<_ISA_, _Tp_>()(), __x, __mask);
		}
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (arithmetic_type<_Tp_>) {
			return __mask ? -__x : __src;
		}
		else if constexpr (std::floating_point<_Type_>) {
			using _XorType = typename IntegerForSizeof<_Type_>::Signed;
			return _Xor<_ISA_, _Type_>()(__x, _Broadcast<_ISA_, _Tp_>()(math::sign_bit<_XorType>()), __mask, __src);
		}
		else {
			return _Sub<_ISA_, _Type_>()(_Zero<_ISA_, _Tp_>()(), __x, __mask, __src);
		}
	}
};

__RAZE_VX_NAMESPACE_END
