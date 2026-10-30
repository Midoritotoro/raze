#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T>
raze_always_inline V all_bits_() noexcept {
	if constexpr (has_avx512f<ISA> && sizeof(V) >= 64) {
		auto z = _mm512_undefined_epi32();
		return as<V>(_mm512_ternarylogic_epi32(z, z, z, 0xff));
	}
	else if constexpr (has_avx512vl<ISA> && sizeof(V) >= 16) {
		if constexpr (sizeof(V) == 32) {
			auto z = _mm256_undefined_si256();
			return as<V>(_mm256_ternarylogic_epi32(z, z, z, 0xff));
		}
		else {
			auto z = _mm_undefined_si128();
			return as<V>(_mm_ternarylogic_epi32(z, z, z, 0xff));
		}
	}
	else if constexpr (has_avx<ISA> && !has_avx2<ISA> && sizeof(V) >= 16) {
		using FV = traits::deduce_simd_vector_type<f32, raze_sizeof_in_bits(V)>;
		return as<V>(equal_<ISA, f32>(zero_<ISA, FV, f32>(), zero_<ISA, FV, f32>()));
	}
	else if constexpr (sizeof(V) >= 16) {
		using IV = traits::deduce_simd_vector_type<i32, raze_sizeof_in_bits(V)>;
		return as<V>(equal_<ISA, i32>(zero_<ISA, IV, i32>(), zero_<ISA, IV, i32>()));
	}
	else {
		using IT = typename IntegerForSizeof<V>::Signed;
		return math::bit_cast<V>(IT(~IT(0)));
	}
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T, raw_mask_type Mask>
raze_always_inline V all_bits_(Mask mask) noexcept {
	if constexpr (has_avx512f<ISA>) {
		auto z = zero_<ISA, V, T>();
		return ternarylogic_<ISA, T>(z, z, z, std::integral_constant<u8, 0xff>{}, mask);
	}

	return select_<ISA, T>(all_bits_<ISA, V, T>(), mask);
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T, raw_mask_type Mask>
raze_always_inline V all_bits_(Mask mask, V src) noexcept {
	return select_<ISA, T>(all_bits_<ISA, V, T>(), src, mask);
}

__RAZE_VX_NAMESPACE_END
