#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, class V>
concept native_ternarylogic = intrin_type<V> && ((has_avx512f<ISA> && sizeof(V) == 64) || 
	(has_avx512vl<ISA> && (sizeof(V) == 32 || sizeof(V) == 16)));

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, u8 Op>
raze_always_inline V ternarylogic_(V x, V y, V z, std::integral_constant<u8, Op> op) noexcept;

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, u8 Op>
raze_always_inline V ternarylogic_(V x, V y, V z, std::integral_constant<u8, Op> op) noexcept;

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M, u8 Op>
raze_always_inline V ternarylogic_(V x, V y, V z, std::integral_constant<u8, Op> op, M mask, V src) noexcept;

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V bit_not_(V x) noexcept {
	if constexpr (native_ternarylogic<ISA, V>) return ternarylogic_<ISA, T>(x, x, x, std::integral_constant<u8, 0x55>{});
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x)));
	}
	else return bit_xor_<ISA, T>(x, all_ones_<ISA, V>());
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type	V, raw_mask_type	M>
raze_always_inline V bit_not_(V x, M mask) noexcept {
	if constexpr (native_ternarylogic<ISA, V>) return ternarylogic_<ISA, T>(x, x, x, std::integral_constant<u8, 0x55>{}, mask);
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return mask ? math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x))) : 0;
	}
	else return bit_xor_<ISA, T>(x, all_ones_<ISA, V>(), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type	V, raw_mask_type	M>
raze_always_inline V bit_not_(V x, M mask, V src) noexcept {
	if constexpr (native_ternarylogic<ISA, V>) return ternarylogic_<ISA, T>(x, x, x, std::integral_constant<u8, 0x55>{}, mask, src);
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return mask ? math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x))) : src;
	}
	else return bit_xor_<ISA, T>(x, all_ones_<ISA, V>(), mask, src);
}

__RAZE_VX_NAMESPACE_END
