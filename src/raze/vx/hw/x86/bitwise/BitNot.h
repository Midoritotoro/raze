#pragma once 

#include <src/raze/vx/hw/x86/construct/AllBits.h>
#include <src/raze/vx/hw/x86/bitwise/BitXor.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, class V>
concept native_ternarylogic = intrin_type<V> && ((has_avx512f<ISA> && sizeof(V) == 64) || 
	(has_avx512vl<ISA> && (sizeof(V) == 32 || sizeof(V) == 16)));

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V bit_not_(V x) noexcept {
	if constexpr (native_ternarylogic<ISA, V>) {
		if constexpr (sizeof(V) == 64) return as<V>(_mm512_ternarylogic_epi32(as<__m512i>(x), as<__m512i>(x), as<__m512i>(x), 0x55));
		else if constexpr (sizeof(V) == 32) return as<V>(_mm256_ternarylogic_epi32(as<__m256i>(x), as<__m256i>(x), as<__m256i>(x), 0x55));
		else return as<V>(_mm_ternarylogic_epi32(as<__m128i>(x), as<__m128i>(x), as<__m128i>(x), 0x55));
	}
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x)));
	}
	else return bit_xor_<ISA, T>(x, all_bits_<ISA, V, T>());
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type	V, raw_mask_type	M>
raze_always_inline V bit_not_(V x, M mask) noexcept {
	if constexpr (native_ternarylogic<ISA, V> && std::is_integral_v<M> && sizeof(T) >= 4) {
		if constexpr (sizeof(V) == 64) {
			if constexpr (sizeof(T) == 8) return as<V>(_mm512_maskz_ternarylogic_epi64(mask, as<__m512i>(x), as<__m512i>(x), as<__m512i>(x), 0x55));
			else return as<V>(_mm512_maskz_ternarylogic_epi32(mask, as<__m512i>(x), as<__m512i>(x), as<__m512i>(x), 0x55));
		}
		else if constexpr (sizeof(V) == 32) {
			if constexpr (sizeof(T) == 8) return as<V>(_mm256_maskz_ternarylogic_epi64(mask, as<__m256i>(x), as<__m256i>(x), as<__m256i>(x), 0x55));
			else return as<V>(_mm256_maskz_ternarylogic_epi32(mask, as<__m256i>(x), as<__m256i>(x), as<__m256i>(x), 0x55));
		}
		else {
			if constexpr (sizeof(T) == 8) return as<V>(_mm_maskz_ternarylogic_epi64(mask, as<__m128i>(x), as<__m128i>(x), as<__m128i>(x), 0x55));
			else return as<V>(_mm_maskz_ternarylogic_epi32(mask, as<__m128i>(x), as<__m128i>(x), as<__m128i>(x), 0x55));
		}
	}
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return mask ? math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x))) : static_cast<V>(0);
	}
	else return bit_xor_<ISA, T>(x, all_bits_<ISA, V, T>(), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type	V, raw_mask_type	M>
raze_always_inline V bit_not_(V x, M mask, V src) noexcept {
	if constexpr (native_ternarylogic<ISA, V>) {
		return select_<ISA, T>(bit_not_<ISA, T>(x), src, mask);
	}
	else if constexpr (arithmetic_type<V>) {
		using Unsigned = typename IntegerForSizeof<V>::Unsigned;
		return mask ? math::bit_cast<V>(Unsigned(~math::bit_cast<Unsigned>(x))) : src;
	}
	else return bit_xor_<ISA, T>(x, all_bits_<ISA, V, T>(), mask, src);
}

__RAZE_VX_NAMESPACE_END

