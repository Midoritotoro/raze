#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/math/SignBit.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V negate_(V x) noexcept {
	if constexpr (arithmetic_type<V>) return -x;
	else if constexpr (std::floating_point<T>) return bit_xor_<ISA, T>(x, broadcast_<ISA, V>(
		math::sign_bit<typename IntegerForSizeof<T>::Signed>()));
	else return sub_<ISA, T>(zero_<ISA, V>(), x);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V negate_(V x, M mask) noexcept {
	if constexpr (arithmetic_type<V>) return mask ? -x : 0;
	else if constexpr (std::floating_point<T>) return bit_xor_<ISA, T>(x, broadcast_<ISA, V>(
		math::sign_bit<typename IntegerForSizeof<T>::Signed>()), mask);
	else return sub_<ISA, T>(zero_<ISA, V>(), x, mask);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V negate_(V x, M mask, V src) noexcept {
	if constexpr (arithmetic_type<V>) return mask ? -x : src;
	else if constexpr (std::floating_point<T>) return bit_xor_<ISA, T>(x, broadcast_<ISA, V>(
		math::sign_bit<typename IntegerForSizeof<T>::Signed>()), mask, src);
	else return sub_<ISA, T>(zero_<ISA, V>(), x, mask, src);
}

__RAZE_VX_NAMESPACE_END
